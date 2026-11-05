import sys
import numpy as np
from gymnasium.utils.env_checker import check_env

from learning_env import Karate_Champ_Env


def make_env(**kw):
    env = Karate_Champ_Env(**kw)
    env.reset(seed=0)
    return env


def test_gym_api():
    check_env(Karate_Champ_Env())


def test_observations_stay_in_bounds():
    env = make_env()
    for _ in range(400):
        obs, _, term, trunc, _ = env.step(env.action_space.sample())
        assert env.observation_space.contains(obs), obs
        if term or trunc:
            env.reset()


def test_same_seed_same_episode():
    def run(seed):
        env = Karate_Champ_Env()
        env.reset(seed=seed)
        env.action_space.seed(seed)
        out = []
        for _ in range(100):
            obs, r, term, trunc, _ = env.step(env.action_space.sample())
            out.append((obs.tobytes(), r))
            if term or trunc:
                break
        return out

    assert run(7) == run(7)


def test_hit_lands_only_once():
    env = make_env()
    p1, p2 = env.player_1, env.player_2
    p1.x, p2.x = 30, 40
    p1.current_state, p1.current_frame = 3, 3
    assert env._collide() == (0, 8)
    p1.current_frame = 4
    assert env._collide() == (0, 0)
    assert p2.health == 92


def test_block_reduces_damage():
    env = make_env()
    p1, p2 = env.player_1, env.player_2
    p1.x, p2.x = 30, 40
    p2.current_state = 6
    p1.current_state, p1.current_frame = 3, 3
    assert env._collide() == (0, 2)
    assert p2.health == 98


def test_health_never_negative():
    env = make_env()
    p1, p2 = env.player_1, env.player_2
    p1.x, p2.x = 30, 40
    p2.health = 3
    p1.current_state, p1.current_frame = 3, 3
    env._collide()
    assert p2.health == 0


def test_opponent_closes_in_and_hurts_idle_agent():
    # regression: P1 used to stop 20px away and whiff forever
    env = make_env()
    for _ in range(300):
        _, _, term, _, info = env.step(0)
        if term:
            break
    assert env.player_2.health < 100, "idle agent never took damage"


def test_opponent_only_attacks_in_reach():
    env = make_env()
    for _ in range(300):
        env.step(0)
        p1 = env.player_1
        if p1.current_state in (3, 4, 8, 9) and p1.current_frame == 0:
            assert env.player_2.x - p1.x <= 16 + 1, "attacked from too far away"


def test_players_cannot_walk_through_each_other():
    env = make_env()
    closest = 999
    for _ in range(600):
        env.step(2)  # agent walks forward the whole time
        closest = min(closest, env.player_2.x - env.player_1.x)
    assert closest >= 11, closest


def test_agent_can_damage_opponent():
    env = make_env()
    for _ in range(300):
        close = env.player_2.x - env.player_1.x <= 14
        _, _, term, _, _ = env.step(3 if close else 2)  # walk up, then punch
        if term:
            break
    assert env.player_1.health < 100, "agent could never hurt the opponent"


def test_both_players_move_every_walk_step():
    env = make_env()
    env.player_1.x, env.player_2.x = 20, 100
    x2 = env.player_2.x
    env.step(2)                      # agent walks forward (left)
    assert env.player_2.x == x2 - 1
    x2 = env.player_2.x
    env.step(1)                      # one step of walking backward (right)
    assert env.player_2.x == x2 + 1


def test_agent_moves_even_with_changing_actions():
    # regression: walking used to need 3 identical actions in a row
    env = make_env()
    env.player_1.x = 4
    start = env.player_2.x
    for a in (2, 0, 2, 0, 2, 0):     # forward / idle alternating
        env.step(a)
    assert env.player_2.x < start


def test_attack_locks_then_releases():
    env = make_env()
    env.step(3)                                # agent starts a punch
    assert env.player_2.locked and env.player_2.current_state == 3
    _, _, _, _, info = env.step(0)             # ignored while locked
    assert info["action_applied"] is False
    assert env.player_2.current_state == 3
    for _ in range(3):
        env.step(0)                            # 5 ticks total
    assert env.player_2.current_state == 0 and not env.player_2.locked


def test_truncates_at_max_steps():
    env = make_env(max_steps=10)
    n, trunc, term = 0, False, False
    while not (trunc or term):
        _, _, term, trunc, _ = env.step(0)
        n += 1
    assert not term and trunc and n == 10


def test_rgb_array_render():
    env = make_env(render_mode="rgb_array")
    img = env.render()
    assert img.shape == (64 * 6, 128 * 6, 3) and img.dtype == np.uint8
    assert img.any(), "frame is completely black"
    env.close()


TESTS = [v for k, v in sorted(globals().items()) if k.startswith("test_")]


def run_tests():
    failed = 0
    for t in TESTS:
        try:
            t()
            print(f"PASS  {t.__name__}")
        except Exception as e:  # noqa: BLE001
            failed += 1
            print(f"FAIL  {t.__name__}: {type(e).__name__}: {e}")
    print(f"\n{len(TESTS) - failed}/{len(TESTS)} passed")
    return failed


def watch():
    env = Karate_Champ_Env(render_mode="human", show_hitboxes=True)
    obs, _ = env.reset(seed=1)
    rng = np.random.default_rng(1)
    while True:
        # demo policy for P2 (a purely random agent is locked in attack animations
        # most of the time, so it hardly looks like it moves): walk in, then fight
        dist = env.player_2.x - env.player_1.x
        if dist > 15:
            action = 2
        elif dist < 12 and rng.random() < 0.3:
            action = 1
        else:
            action = int(rng.choice([3, 4, 6, 8]))
        obs, r, term, trunc, _ = env.step(action)
        if term or trunc:
            obs, _ = env.reset()


if __name__ == "__main__":
    if "--watch" in sys.argv:
        watch()
    else:
        sys.exit(1 if run_tests() else 0)