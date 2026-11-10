"""
Adversarial self-play PPO (pure numpy) for Karate Champ.

* One shared policy plays BOTH seats (obs is mirrored, seat is an input).
* Zero-sum reward: (damage I deal - damage I take)/10 + KO/timeout bonus (same as env).
* League: opponents are the latest snapshot, random older snapshots, and the
  scripted P1 heuristic (so the agent stays good vs. the game's real baseline).
* Policy net (exported to Arduino): 29 -> HID (ReLU) -> 10.   Value net is bigger, training only.
"""
import argparse, time, os, pickle
import numpy as np
from karate_sim import *

# ------------------------------------------------------------------ nets
class MLP:
    def __init__(s, sizes, rng, last_gain=1.0):
        s.W, s.b = [], []
        for i in range(len(sizes) - 1):
            g = np.sqrt(2.0) if i < len(sizes) - 2 else last_gain
            s.W.append((rng.standard_normal((sizes[i], sizes[i + 1])) * g / np.sqrt(sizes[i])).astype(np.float32))
            s.b.append(np.zeros(sizes[i + 1], np.float32))

    def forward(s, x):
        hs, h, L = [x], x, len(s.W)
        for i in range(L):
            z = h @ s.W[i] + s.b[i]
            h = np.maximum(z, 0) if i < L - 1 else z
            hs.append(h)
        return hs

    def backward(s, hs, d):
        L = len(s.W); gW = [None] * L; gb = [None] * L
        for i in reversed(range(L)):
            gW[i] = hs[i].T @ d
            gb[i] = d.sum(0)
            if i > 0:
                d = (d @ s.W[i].T) * (hs[i] > 0)
        return gW + gb

    def params(s): return s.W + s.b
    def snapshot(s): return ([w.copy() for w in s.W], [b.copy() for b in s.b])


class Frozen:
    def __init__(s, snap): s.W, s.b = snap
    forward = MLP.forward


class Adam:
    def __init__(s, params):
        s.m = [np.zeros_like(p) for p in params]; s.v = [np.zeros_like(p) for p in params]; s.t = 0

    def step(s, params, grads, lr, clip=0.5):
        gn = np.sqrt(sum(float((g * g).sum()) for g in grads))
        if gn > clip:
            grads = [g * (clip / gn) for g in grads]
        s.t += 1
        c1, c2 = 1 - 0.9 ** s.t, 1 - 0.999 ** s.t
        for p, g, m, v in zip(params, grads, s.m, s.v):
            m *= 0.9; m += 0.1 * g
            v *= 0.999; v += 0.001 * g * g
            p -= lr * (m / c1) / (np.sqrt(v / c2) + 1e-8)

# ------------------------------------------------------------------ agents
class NetAgent:
    def __init__(s, net, greedy=False, eps=0.0): s.net, s.greedy, s.eps = net, greedy, eps
    def act(s, X, fights, seats, rng):
        lg = s.net.forward(X)[-1]
        a = lg.argmax(1) if s.greedy else (lg + rng.gumbel(size=lg.shape)).argmax(1)
        if s.eps:
            m = rng.random(len(a)) < s.eps
            a = np.where(m, rng.integers(0, N_ACTIONS, len(a)), a)
        return a

class Scripted:
    def act(s, X, fights, seats, rng):
        return np.array([heuristic(f, sd) for f, sd in zip(fights, seats)])

class Constant:
    def __init__(s, a): s.a = a
    def act(s, X, fights, seats, rng): return np.full(len(fights), s.a)

class RandomAgent:
    def act(s, X, fights, seats, rng): return rng.integers(0, N_ACTIONS, len(fights))


def obs_batch(fights, seats):
    return np.array([features(f, s) for f, s in zip(fights, seats)], np.float32) / 127.0


def evaluate(agent, opp, n=64, seed=123, max_steps=500, full=False):
    """A (as seat 0 in half the games, seat 1 in the other half) vs B. Score: win=1 draw=.5 loss=0."""
    rng = np.random.default_rng(seed)
    fights = [Fight(max_steps=max_steps, seed=seed + i) for i in range(n)]
    seatA = [i % 2 for i in range(n)]
    seatB = [1 - s for s in seatA]
    alive = list(range(n))
    while alive:
        fs = [fights[i] for i in alive]
        XA = obs_batch(fs, [seatA[i] for i in alive]); XB = obs_batch(fs, [seatB[i] for i in alive])
        aA = agent.act(XA, fs, [seatA[i] for i in alive], rng)
        aB = opp.act(XB, fs, [seatB[i] for i in alive], rng)
        nxt = []
        for k, i in enumerate(alive):
            a = [0, 0]; a[seatA[i]] = int(aA[k]); a[seatB[i]] = int(aB[k])
            fights[i].step(a[0], a[1])
            if not (fights[i].term or fights[i].trunc): nxt.append(i)
        alive = nxt
    score, kos = 0.0, 0
    for i, f in enumerate(fights):
        r = f.res if seatA[i] == 0 else -f.res
        score += 1.0 if r > 0 else (0.5 if r == 0 else 0.0)
        kos += f.term
    return (score / n, kos / n) if full else score / n

# ------------------------------------------------------------------ training
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--iters", type=int, default=1500)
    ap.add_argument("--hid", type=int, default=32)
    ap.add_argument("--envs", type=int, default=32)
    ap.add_argument("--T", type=int, default=128)
    ap.add_argument("--lr", type=float, default=7e-4)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--out", default="run")
    ap.add_argument("--frozen", default=None, help="train an EXPLOITER against this frozen pickle (greedy)")
    ap.add_argument("--p_script", type=float, default=0.10)
    ap.add_argument("--p_bots", type=float, default=0.15, help="idle/random/block/walk bots")
    ap.add_argument("--rep_pen", type=float, default=4.0, help="penalty (env reward units) for starting the same attack type twice in a row")
    ap.add_argument("--init", default=None, help="warm-start policy from a snapshot pickle (29-input v1 nets are zero-padded)")
    ap.add_argument("--vf_warmup", type=int, default=0, help="iterations to train only the value net first")
    ap.add_argument("--ent0", type=float, default=0.012, help="initial entropy bonus extra (decays to 0)")
    ap.add_argument("--draw_pen", type=float, default=6.0, help="penalty (env reward units) for a drawn timeout")
    ap.add_argument("--snap_every", type=int, default=15)
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    rng = np.random.default_rng(a.seed)
    gamma, lam, clip_eps, epochs, mbs = 0.97, 0.95, 0.2, 4, 512

    if a.init:
        with open(a.init, "rb") as fh: iW, ib = pickle.load(fh)
        a.hid = iW[0].shape[1]
        pi = MLP([N_FEATURES, a.hid, N_ACTIONS], rng)
        W1 = np.zeros((N_FEATURES, a.hid), np.float32); W1[:iW[0].shape[0]] = iW[0]   # new inputs start at weight 0
        pi.W = [W1, iW[1].copy()]; pi.b = [ib[0].copy(), ib[1].copy()]
    else:
        pi = MLP([N_FEATURES, a.hid, N_ACTIONS], rng, last_gain=0.01)
    vf = MLP([N_FEATURES, 64, 64, 1], rng, last_gain=1.0)
    opt_pi, opt_vf = Adam(pi.params()), Adam(vf.params())
    pool = [pi.snapshot()]
    frozen_agent = None
    if a.frozen:
        with open(a.frozen, "rb") as fh: frozen_agent = NetAgent(Frozen(pickle.load(fh)), greedy=True)
    scripted, rand_agent = Scripted(), RandomAgent()

    def new_opp():
        if frozen_agent is not None: return "frozen", frozen_agent
        u = rng.random()
        if u < a.p_script: return "script", scripted
        if u < a.p_script + a.p_bots:
            k = int(rng.integers(0, 5))
            return "bot", [rand_agent, Constant(IDLE), Constant(BLOCK_R), Constant(WALK_FWD), Constant(WALK_BACK)][k]
        if u < a.p_script + a.p_bots + 0.45 or len(pool) == 1: return "latest", NetAgent(Frozen(pool[-1]))
        return "old", NetAgent(Frozen(pool[rng.integers(0, len(pool) - 1)]))

    E, T = a.envs, a.T
    fights = [Fight(seed=a.seed * 1000 + i) for i in range(E)]
    seats = [int(rng.integers(0, 2)) for _ in range(E)]
    opps = [new_opp() for _ in range(E)]
    stats = {}   # kind -> recent results
    ep_len = []
    t0 = time.time()
    last_eval = None
    atk = np.zeros(4)

    for it in range(1, a.iters + 1):
        frac = 1 - (it - 1) / a.iters
        lr = a.lr * (0.15 + 0.85 * frac)
        ent_c = 0.003 + a.ent0 * frac
        OB = np.zeros((T, E, N_FEATURES), np.float32); AC = np.zeros((T, E), np.int64)
        LP = np.zeros((T, E), np.float32); VA = np.zeros((T, E), np.float32)
        RW = np.zeros((T, E), np.float32); DN = np.zeros((T, E), np.float32); MK = np.zeros((T, E), np.float32)

        for t in range(T):
            Xl = obs_batch(fights, seats)
            lg = pi.forward(Xl)[-1]
            act = (lg + rng.gumbel(size=lg.shape)).argmax(1)
            z = lg - lg.max(1, keepdims=True); lp_all = z - np.log(np.exp(z).sum(1, keepdims=True))
            OB[t] = Xl; AC[t] = act; LP[t] = lp_all[np.arange(E), act]
            VA[t] = vf.forward(Xl)[-1][:, 0]
            MK[t] = [0.0 if f.p[s].locked else 1.0 for f, s in zip(fights, seats)]
            for i in range(E):
                if MK[t, i] and ATTACK[act[i]]: atk[ATK_TYPE[act[i]] - 1] += 1

            opp_seats = [1 - s for s in seats]
            Xo = obs_batch(fights, opp_seats)
            oact = np.zeros(E, np.int64)
            groups = {}
            for i, (_, ag) in enumerate(opps): groups.setdefault(id(ag), (ag, []))[1].append(i)
            for ag, idx in groups.values():
                oact[idx] = ag.act(Xo[idx], [fights[i] for i in idx], [opp_seats[i] for i in idx], rng)

            for i in range(E):
                f, sd = fights[i], seats[i]
                a12 = [0, 0]; a12[sd] = int(act[i]); a12[1 - sd] = int(oact[i])
                d1, d2 = f.step(a12[0], a12[1])
                dealt, taken = (d1, d2) if sd == 0 else (d2, d1)
                r = (dealt - taken + f.terminal_bonus(sd) - (a.draw_pen if (f.trunc and f.res == 0) else 0.0)
                     - (a.rep_pen if f.rep[sd] else 0.0)) / 10.0
                RW[t, i] = r
                if f.term or f.trunc:
                    DN[t, i] = 1.0
                    res = f.res if sd == 0 else -f.res
                    stats.setdefault(opps[i][0], []).append(1.0 if res > 0 else (0.5 if res == 0 else 0.0))
                    ep_len.append(f.steps)
                    f.reset(); seats[i] = int(rng.integers(0, 2)); opps[i] = new_opp()

        last_v = vf.forward(obs_batch(fights, seats))[-1][:, 0]
        adv = np.zeros((T, E), np.float32); g = np.zeros(E, np.float32)
        for t in reversed(range(T)):
            nv = last_v if t == T - 1 else VA[t + 1]
            nn = 1.0 - DN[t]
            delta = RW[t] + gamma * nv * nn - VA[t]
            g = delta + gamma * lam * nn * g
            adv[t] = g
        ret = adv + VA

        X = OB.reshape(-1, N_FEATURES); A = AC.reshape(-1); OLP = LP.reshape(-1)
        ADV = adv.reshape(-1); RET = ret.reshape(-1); M = MK.reshape(-1)
        valid = M > 0
        ADV = (ADV - ADV[valid].mean()) / (ADV[valid].std() + 1e-8)
        n = len(X)
        for _ in range(epochs):
            perm = rng.permutation(n)
            for s0 in range(0, n, mbs):
                ix = perm[s0:s0 + mbs]
                xb, ab, lpb, advb, retb, mb = X[ix], A[ix], OLP[ix], ADV[ix], RET[ix], M[ix]
                hs = pi.forward(xb); lg = hs[-1]
                z = lg - lg.max(1, keepdims=True); p = np.exp(z); p /= p.sum(1, keepdims=True)
                lpa = np.log(p + 1e-8)
                ratio = np.exp(lpa[np.arange(len(ix)), ab] - lpb)
                active = ~(((advb > 0) & (ratio > 1 + clip_eps)) | ((advb < 0) & (ratio < 1 - clip_eps)))
                w = mb / max(mb.sum(), 1.0)
                onehot = np.zeros_like(p); onehot[np.arange(len(ix)), ab] = 1
                dlg = (-advb * ratio * active * w)[:, None] * (onehot - p)
                H = -(p * lpa).sum(1)
                dlg += ent_c * w[:, None] * p * (lpa + H[:, None])
                if it > a.vf_warmup:
                    opt_pi.step(pi.params(), pi.backward(hs, dlg.astype(np.float32)), lr)

                hv = vf.forward(xb)
                dv = ((hv[-1][:, 0] - retb) / len(ix))[:, None].astype(np.float32)
                opt_vf.step(vf.params(), vf.backward(hv, dv), lr, clip=1.0)

        if it % a.snap_every == 0:
            pool.append(pi.snapshot())
            if len(pool) > 40: pool.pop(1)

        if it % 25 == 0 or it == a.iters:
            greedy = NetAgent(pi, greedy=True)
            ev = {"script": evaluate(greedy, scripted), "random": evaluate(greedy, rand_agent), "idle": evaluate(greedy, Constant(IDLE)),
                  "oldsnap": evaluate(greedy, NetAgent(Frozen(pool[max(0, len(pool) - 8)]), greedy=True))}
            if frozen_agent is not None: ev["frozen"] = evaluate(greedy, frozen_agent)
            tr = " ".join(f"{k}:{np.mean(v[-100:]):.2f}" for k, v in sorted(stats.items()))
            print(f"it {it:4d} | {time.time()-t0:5.0f}s | eplen {np.mean(ep_len[-100:]):5.0f} | train-WR {tr} | "
                  f"atk% P/FK/BK/SW {' '.join(f'{100*x/max(atk.sum(),1):.0f}' for x in atk)} | "
                  f"greedy-eval " + " ".join(f"{k}:{v:.2f}" for k, v in ev.items()), flush=True)
            atk[:] = 0
            with open(f"{a.out}/ckpt_{it:05d}.pkl", "wb") as fh: pickle.dump(pi.snapshot(), fh)
    with open(f"{a.out}/final.pkl", "wb") as fh: pickle.dump(pi.snapshot(), fh)
    with open(f"{a.out}/pool.pkl", "wb") as fh: pickle.dump(pool, fh)

if __name__ == "__main__":
    main()