import gymnasium as gym
from gymnasium import spaces
import numpy as np

# modules
from Model.player import Player
from oled_emulator import OledRenderer

SCREEN_WIDTH = 128
SCREEN_HEIGHT = 64
RENDER_SCALE = 6
ROUND_SECONDS = 90
MIN_X, MAX_X = 4, 110
MIN_GAP = 11
WALK_SPEED = 1

# scripted opponent (player 1)
P1_ATTACK_RANGE = 16
P1_ATTACKS = [3, 4, 8]
P1_ATTACK_PROBS = [0.5, 0.3, 0.2]
MAX_HEALTH = 100
MAX_FRAME = 6

# state ids
IDLE, WALK_BACK, WALK_FWD, PUNCH, FRONT_KICK, BACK_KICK = 0, 1, 2, 3, 4, 5
BLOCK_R, BLOCK_L, SWEEP_L, SWEEP_R = 6, 7, 8, 9

ANIM_LEN = {
    0: 1,  # idle
    1: 3,  # walk backward
    2: 3,  # walk forward
    3: 5,  # punch
    4: 5,  # front kick
    5: 7,  # back kick
    6: 4,  # block right
    7: 4,  # block left
    8: 6,  # sweep left
    9: 6,  # sweep right
}

LOOPING_STATES = {0, 1, 2, 6, 7}
ATTACK_STATES = {3, 4, 5, 8, 9}
BLOCK_STATES = {6, 7}


class Karate_Champ_Env(gym.Env):

    metadata = {"render_modes": ["human", "rgb_array"], "render_fps": 6}

    def __init__(
        self,
        max_steps=500,
        win_bonus=20.0,
        timeout_bonus=10.0,
        render_mode=None,
        render_scale=RENDER_SCALE,
    ):
        super().__init__()

        self.max_steps = max_steps
        self.steps = 0
        self.win_bonus = win_bonus
        self.timeout_bonus = timeout_bonus
        assert render_mode is None or render_mode in self.metadata["render_modes"]
        self.render_mode = render_mode

        self._render_scale = render_scale
        self._renderer = None

        # action space
        self.action_space = spaces.Discrete(10)

        # observation space
        # [p2_hp, p2_x, p2_frame, p2_state, p1_hp, p1_x, p1_frame, p1_state, rel_dist]
        self.observation_space = spaces.Box(
            low=np.array([0, 0, 0, 0, 0, 0, 0, 0, -1], dtype=np.float32),
            high=np.array([1, 1, 1, 1, 1, 1, 1, 1, 1], dtype=np.float32),
            dtype=np.float32,
        )

        self._landed = [False, False]

        self.player_1: Player = Player(x=30)
        self.player_2: Player = Player(x=100)
        self._init_players()

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)

        self.steps = 0
        self._init_players()

        if self.render_mode == "human":
            self.render()

        return self._obs(), {}

    def step(self, action):
        action = int(action)

        # inputs
        action_applied = self._set_state(self.player_2, 1, action)
        self._set_state(self.player_1, 0, self._p1_heuristic())

        # collision
        dmg_dealt, dmg_taken = self._collide()

        # movement
        self._move(self.player_1, self.player_2, faces_right=True)
        self._move(self.player_2, self.player_1, faces_right=False)

        # advance animations
        self._tick_anim(self.player_1)
        self._tick_anim(self.player_2)

        # bookkeeping
        if self.get_attack_damage(self.player_1) == 0:
            self._landed[0] = False
        if self.get_attack_damage(self.player_2) == 0:
            self._landed[1] = False

        self.steps += 1

        reward = float(dmg_dealt - dmg_taken)

        p1_dead = self.player_1.health <= 0
        p2_dead = self.player_2.health <= 0
        terminated = p1_dead or p2_dead
        truncated = (not terminated) and self.steps >= self.max_steps

        if terminated:
            if p1_dead and not p2_dead:
                reward += self.win_bonus
            elif p2_dead and not p1_dead:
                reward -= self.win_bonus
        elif truncated:
            if self.player_2.health > self.player_1.health:
                reward += self.timeout_bonus
            elif self.player_1.health > self.player_2.health:
                reward -= self.timeout_bonus

        info = {
            "action_applied": action_applied,
            "p1_health": self.player_1.health,
            "p2_health": self.player_2.health,
        }

        if self.render_mode == "human":
            self.render()

        return self._obs(), reward, terminated, truncated, info

    def render(self):
        if self.render_mode is None:
            return None

        if self._renderer is None:
            self._renderer = OledRenderer(
                scale=self._render_scale, fps=self.metadata["render_fps"]
            )

        # round timer shown on the OLED, derived from elapsed steps
        remaining = int(np.ceil(ROUND_SECONDS * (1 - self.steps / self.max_steps)))
        timer = min(ROUND_SECONDS, max(0, remaining))

        fb = self._renderer.draw_frame(self.player_1, self.player_2, timer)

        if self.render_mode == "rgb_array":
            return self._renderer.to_rgb(fb)

        self._renderer.show(fb)
        return None

    def close(self):
        if self._renderer is not None:
            self._renderer.close()
            self._renderer = None

    # helpers
    def _init_players(self):
        self.player_1 = Player(x=30)
        self.player_2 = Player(x=100)
        for p in (self.player_1, self.player_2):
            p.health = MAX_HEALTH
            p.current_state = IDLE
            p.current_frame = 0
            p.locked = False
        self._landed = [False, False]

    def _obs(self):
        p1, p2 = self.player_1, self.player_2
        return np.array(
            [
                # player 2 (AI)
                max(0, p2.health) / MAX_HEALTH,
                p2.x / MAX_X,
                p2.current_frame / MAX_FRAME,
                p2.current_state / 9.0,

                # player 1
                max(0, p1.health) / MAX_HEALTH,
                p1.x / MAX_X,
                p1.current_frame / MAX_FRAME,
                p1.current_state / 9.0,

                # relative distance
                np.clip((p2.x - p1.x) / MAX_X, -1.0, 1.0),
            ],
            dtype=np.float32,
        )

    def _set_state(self, player, idx, new_state):
        if player.locked:
            return False

        if new_state != player.current_state:
            player.current_frame = 0
            self._landed[idx] = False

        player.current_state = new_state

        if new_state in ATTACK_STATES:
            player.locked = True

        return True

    @staticmethod
    def _get_hurtbox(player: Player):
        return player.x + 1, SCREEN_HEIGHT - 22, 11, 22

    @staticmethod
    def _get_hitbox(player: Player, faces_right: bool):
        s, f, x = player.current_state, player.current_frame, player.x

        if s == PUNCH and f in (3, 4):
            return (x + 13 if faces_right else x - 8), SCREEN_HEIGHT - 18, 8, 8
        if s == FRONT_KICK and f in (2, 3):
            return (x + 10 if faces_right else x - 10), SCREEN_HEIGHT - 20, 10, 10
        if s == BACK_KICK and f == 3:
            return (x - 8 if faces_right else x + 13), SCREEN_HEIGHT - 16, 10, 8
        if s in (SWEEP_L, SWEEP_R) and 1 <= f <= 3:
            return (x + 13 if faces_right else x - 17), SCREEN_HEIGHT - 12, 17, 12
        return None

    @staticmethod
    def _aabb(a, b):
        ax, ay, aw, ah = a
        bx, by, bw, bh = b
        return ax < bx + bw and ax + aw > bx and ay < by + bh and ay + ah > by

    @staticmethod
    def get_attack_damage(player: Player):
        s, f = player.current_state, player.current_frame
        if s == PUNCH and f in (3, 4):
            return 8
        if s == FRONT_KICK and f in (2, 3):
            return 12
        if s == BACK_KICK and f == 3:
            return 15
        if s in (SWEEP_L, SWEEP_R) and 1 <= f <= 3:
            return 10
        return 0

    def _apply_hit(self, attacker: Player, defender: Player):
        dmg = self.get_attack_damage(attacker)
        if dmg <= 0:
            return 0
        if defender.current_state in BLOCK_STATES:
            dmg = max(1, dmg // 4)
        before = defender.health
        defender.health = max(0, defender.health - dmg)
        return before - defender.health

    def _collide(self):
        p1, p2 = self.player_1, self.player_2

        p1_hit = self._get_hitbox(p1, faces_right=True)
        p2_hit = self._get_hitbox(p2, faces_right=False)
        p1_hurt = self._get_hurtbox(p1)
        p2_hurt = self._get_hurtbox(p2)

        p1_connects = (
            p1_hit is not None and not self._landed[0] and self._aabb(p1_hit, p2_hurt)
        )
        p2_connects = (
            p2_hit is not None and not self._landed[1] and self._aabb(p2_hit, p1_hurt)
        )

        dmg_dealt = 0  # agent (P2) -> P1
        dmg_taken = 0  # P1 -> agent (P2)

        if p2_connects:
            dmg_dealt = self._apply_hit(p2, p1)
            self._landed[1] = True

        if p1_connects:
            dmg_taken = self._apply_hit(p1, p2)
            self._landed[0] = True

        return dmg_dealt, dmg_taken

    @staticmethod
    def _tick_anim(player: Player):
        state = player.current_state
        length = ANIM_LEN[state]

        if state in LOOPING_STATES:
            if state == WALK_BACK:
                player.current_frame = (player.current_frame + length - 1) % length
            else:
                player.current_frame = (player.current_frame + 1) % length
            return

        player.current_frame += 1
        if player.current_frame >= length:
            # attack finished
            player.current_frame = 0
            player.current_state = IDLE
            player.locked = False

    @staticmethod
    def _move(player: Player, other: Player, faces_right: bool):
        if player.current_state == WALK_FWD:
            direction = 1 if faces_right else -1
        elif player.current_state == WALK_BACK:
            direction = -1 if faces_right else 1
        else:
            return

        new_x = min(MAX_X, max(MIN_X, player.x + direction * WALK_SPEED))

        if player.current_state == WALK_FWD:
            if faces_right:
                new_x = max(player.x, min(new_x, other.x - MIN_GAP))
            else:
                new_x = min(player.x, max(new_x, other.x + MIN_GAP))

        player.x = new_x

    def _p1_heuristic(self):
        dist = self.player_2.x - self.player_1.x

        if dist > P1_ATTACK_RANGE:
            return WALK_FWD

        return int(self.np_random.choice(P1_ATTACKS, p=P1_ATTACK_PROBS))