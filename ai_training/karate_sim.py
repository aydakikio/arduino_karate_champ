"""
Fast, symmetric (two-agent) re-implementation of Karate_Champ_Env physics.
Pure Python ints, no gymnasium/numpy needed.  Verified step-for-step against the
original env in test_equivalence.py.

Observation (from the point of view of "me"), all ints in 0..127 so the exact same
vector can be built on the Arduino from its own game variables:
  0 my_hp        1 opp_hp       2 dist(p2.x-p1.x)   3 my_dist_to_own_wall
  4 opp_dist_to_own_wall        5 my_frame*20       6 opp_frame*20
  7 time_left (0..127)          8 me_is_player_1 (127/0)
  9..18  one-hot(my_state)*127  19..28 one-hot(opp_state)*127
  29..32 one-hot(my LAST attack type: punch, front kick, back kick, sweep)*127 (all 0 = none yet)
"""
import random

SCREEN_HEIGHT = 64
MIN_X, MAX_X = 4, 110
MIN_GAP = 11
WALK_SPEED = 1
MAX_HEALTH = 100
P1_ATTACK_RANGE = 16
P1_ATTACKS = (3, 4, 8)
P1_ATTACK_PROBS = (0.5, 0.3, 0.2)

(IDLE, WALK_BACK, WALK_FWD, PUNCH, FRONT_KICK, BACK_KICK,
 BLOCK_R, BLOCK_L, SWEEP_L, SWEEP_R) = range(10)
N_ACTIONS = 10
N_FEATURES = 33
ANIM_LEN = (1, 3, 3, 5, 5, 7, 4, 4, 6, 6)
LOOPING = (1, 1, 1, 0, 0, 0, 1, 1, 0, 0)
ATTACK = (0, 0, 0, 1, 1, 1, 0, 0, 1, 1)
BLOCK = (0, 0, 0, 0, 0, 0, 1, 1, 0, 0)
ATK_TYPE = (0, 0, 0, 1, 2, 3, 0, 0, 4, 4)   # 1 punch, 2 front kick, 3 back kick, 4 sweep (L/R are the same)


class Fighter:
    __slots__ = ("x", "hp", "st", "fr", "locked", "landed", "last")

    def __init__(self, x):
        self.x = x
        self.hp = MAX_HEALTH
        self.st = IDLE
        self.fr = 0
        self.locked = False
        self.landed = False
        self.last = 0          # type of the last attack this fighter started (0 = none)


def attack_damage(st, fr):
    if st == PUNCH and (fr == 3 or fr == 4):
        return 8
    if st == FRONT_KICK and (fr == 2 or fr == 3):
        return 12
    if st == BACK_KICK and fr == 3:
        return 15
    if (st == SWEEP_L or st == SWEEP_R) and 1 <= fr <= 3:
        return 10
    return 0


def hitbox(p, faces_right):
    s, f, x = p.st, p.fr, p.x
    if s == PUNCH and (f == 3 or f == 4):
        return ((x + 13 if faces_right else x - 8), SCREEN_HEIGHT - 18, 8, 8)
    if s == FRONT_KICK and (f == 2 or f == 3):
        return ((x + 10 if faces_right else x - 10), SCREEN_HEIGHT - 20, 10, 10)
    if s == BACK_KICK and f == 3:
        return ((x - 8 if faces_right else x + 13), SCREEN_HEIGHT - 16, 10, 8)
    if (s == SWEEP_L or s == SWEEP_R) and 1 <= f <= 3:
        return ((x + 13 if faces_right else x - 17), SCREEN_HEIGHT - 12, 17, 12)
    return None


def aabb(a, b):
    ax, ay, aw, ah = a
    bx, by, bw, bh = b
    return ax < bx + bw and ax + aw > bx and ay < by + bh and ay + ah > by


class Fight:
    def __init__(self, max_steps=500, win_bonus=20.0, timeout_bonus=10.0, seed=None):
        self.max_steps = max_steps
        self.win_bonus = win_bonus
        self.timeout_bonus = timeout_bonus
        self.rng = random.Random(seed)
        self.reset()

    def reset(self):
        self.p = (Fighter(30), Fighter(100))
        self.steps = 0
        self.term = self.trunc = False
        self.rep = (0, 0)   # 1 if that player just started the same attack type as their previous one
        self.res = 0   # +1: P1 ahead / won, -1: P2 ahead / won, 0: draw (valid once done)

    # -- helpers -----------------------------------------------------------
    @staticmethod
    def _set(p, new):
        if p.locked:
            return 0
        if new != p.st:
            p.fr = 0
            p.landed = False
        p.st = new
        rep = 0
        if ATTACK[new]:
            p.locked = True
            t = ATK_TYPE[new]
            rep = 1 if t == p.last else 0
            p.last = t
        return rep

    @staticmethod
    def _apply(att, dfn):
        dmg = attack_damage(att.st, att.fr)
        if dmg <= 0:
            return 0
        if BLOCK[dfn.st]:
            dmg = max(1, dmg // 4)
        before = dfn.hp
        dfn.hp = max(0, dfn.hp - dmg)
        return before - dfn.hp

    @staticmethod
    def _move(p, other, faces_right):
        if p.st == WALK_FWD:
            d = 1 if faces_right else -1
        elif p.st == WALK_BACK:
            d = -1 if faces_right else 1
        else:
            return
        nx = min(MAX_X, max(MIN_X, p.x + d * WALK_SPEED))
        if p.st == WALK_FWD:
            if faces_right:
                nx = max(p.x, min(nx, other.x - MIN_GAP))
            else:
                nx = min(p.x, max(nx, other.x + MIN_GAP))
        p.x = nx

    @staticmethod
    def _tick(p):
        st = p.st
        n = ANIM_LEN[st]
        if LOOPING[st]:
            p.fr = (p.fr + n - 1) % n if st == WALK_BACK else (p.fr + 1) % n
            return
        p.fr += 1
        if p.fr >= n:
            p.fr = 0
            p.st = IDLE
            p.locked = False

    # -- main step ---------------------------------------------------------
    def step(self, a1, a2):
        """Returns (dmg_by_p1, dmg_by_p2).  Sets self.term / trunc / res."""
        p1, p2 = self.p
        r2 = self._set(p2, a2)
        r1 = self._set(p1, a1)
        self.rep = (r1, r2)

        h1 = hitbox(p1, True)
        h2 = hitbox(p2, False)
        c1 = h1 is not None and not p1.landed and aabb(h1, (p2.x + 1, SCREEN_HEIGHT - 22, 11, 22))
        c2 = h2 is not None and not p2.landed and aabb(h2, (p1.x + 1, SCREEN_HEIGHT - 22, 11, 22))
        d1 = d2 = 0
        if c2:
            d2 = self._apply(p2, p1)
            p2.landed = True
        if c1:
            d1 = self._apply(p1, p2)
            p1.landed = True

        self._move(p1, p2, True)
        self._move(p2, p1, False)
        self._tick(p1)
        self._tick(p2)
        if attack_damage(p1.st, p1.fr) == 0:
            p1.landed = False
        if attack_damage(p2.st, p2.fr) == 0:
            p2.landed = False
        self.steps += 1

        dead1, dead2 = p1.hp <= 0, p2.hp <= 0
        self.term = dead1 or dead2
        self.trunc = (not self.term) and self.steps >= self.max_steps
        self.res = 0
        if self.term:
            if dead1 and not dead2:
                self.res = -1
            elif dead2 and not dead1:
                self.res = 1
        elif self.trunc:
            self.res = (p1.hp > p2.hp) - (p1.hp < p2.hp)
        return d1, d2

    def terminal_bonus(self, seat):
        """Win/timeout bonus from `seat`'s point of view (0 = P1, 1 = P2)."""
        if not (self.term or self.trunc):
            return 0.0
        b = self.win_bonus if self.term else self.timeout_bonus
        return self.res * b if seat == 0 else -self.res * b


def features(f, seat):
    p1, p2 = f.p
    me, op = (p1, p2) if seat == 0 else (p2, p1)
    dist = p2.x - p1.x
    dist = 0 if dist < 0 else (127 if dist > 127 else dist)
    if seat == 0:
        mw, ow = p1.x - MIN_X, MAX_X - p2.x
    else:
        mw, ow = MAX_X - p2.x, p1.x - MIN_X
    v = [0] * N_FEATURES
    v[0] = me.hp if me.hp > 0 else 0
    v[1] = op.hp if op.hp > 0 else 0
    v[2] = dist
    v[3] = mw
    v[4] = ow
    v[5] = me.fr * 20
    v[6] = op.fr * 20
    v[7] = (f.max_steps - f.steps) * 127 // f.max_steps
    v[8] = 127 if seat == 0 else 0
    v[9 + me.st] = 127
    v[19 + op.st] = 127
    if me.last:
        v[28 + me.last] = 127
    return v


def heuristic(f, seat):
    """The env's scripted P1 policy, usable from either seat."""
    p1, p2 = f.p
    if p2.x - p1.x > P1_ATTACK_RANGE:
        return WALK_FWD
    return f.rng.choices(P1_ATTACKS, weights=P1_ATTACK_PROBS)[0]