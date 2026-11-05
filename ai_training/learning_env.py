import gymnasium as gym
from gymnasium import spaces
import numpy as np

#modules
from Model.player import Player

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

class Karate_Champ_Env(gym.Env):
    def __init__(self,max_steps=600):
        super().__init__()

        #max steps
        self.max_steps = max_steps
        self.steps = 0

        #action space
        self.action_space = spaces.Discrete(10)

        #observation space
        #[health - position - frame - state]
        self.observation_space = spaces.Box(
            low=np.array([0,0,4,4,0,0,0,0],dtype=np.float32),
            high=np.array([100,100,110,110,6,6,9,9],dtype=np.float32)
        )

        #init player
        self.player_1 : Player = Player(x=10)
        self.player_2 : Player = Player(x=110)

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)

        self.steps=0

        self.player_1 = Player(x=30)
        self.player_2 = Player(x=100)

        return self._obs(),{}

    def step(self, action):

        self.player_2.current_state = action
        self.player_1.current_state = self._p1_heuristic()

        self._tick_anim(player=self.player_1)
        self._tick_anim(player=self.player_2)

        self._move(player=self.player_1, faces_right=True)
        self._move(player=self.player_2, faces_right=False)

        dmg_dealt, dmg_taken = self._collide()

        reward=dmg_dealt - dmg_taken

        terminated = True if self.player_1.health <=0 or self.player_2.health <= 0 else False
        truncated = True if self.steps >= self.max_steps else False

        self.steps +=1

        return self._obs(), reward, terminated, truncated, {}

    def render(self):
        print()

    def close(self):
        print()

    #helper functions
    def _obs(self):
        return np.array([
            # player 2 (AI)
            self.player_2.health,
            self.player_2.x,
            self.player_2.current_frame,
            self.player_2.current_state,

            # player 1
            self.player_1.health,
            self.player_1.x,
            self.player_1.current_frame,
            self.player_1.current_state,

        ],dtype=np.float32)

    @staticmethod
    def _get_hurtbox(player:Player):
        return player.x + 1, 42, 11, 22

    @staticmethod
    def _get_hitbox(player:Player):
        match player.current_state:
            case 3:
                if player.current_frame == 3 or player.current_frame == 4:
                    return player.x-8, 46, 8, 8
            case 4:
                if player.current_frame == 2 or player.current_frame==3:
                    return player.x -10, 40,10,10
            case 5:
                if player.current_frame ==3:
                    return player.x+13,48,10,8
            case 8 | 9:
                if 1 <= player.current_frame <= 3:
                    return player.x-17,52,17,12
        return None

    @staticmethod
    def _aabb(a, b):
        ax, ay, aw, ah = a
        bx, by, bw, bh = b

        return ax <bx+bw and ax+aw>bx and ay <by+bh and ay+ah>by

    @staticmethod
    def get_attack_damage(player:Player):
        match player.current_state:
            case 3:
                if player.current_frame == 3 or player.current_frame == 4:
                    return 8
            case 4:
                if player.current_frame == 2 or player.current_frame==3:
                    return 12
            case 5:
                if player.current_frame ==3:
                    return 15
            case 8 | 9:
                if 1 <= player.current_frame <= 3:
                    return 10
        return 0

    def _collide(self):

        #get hit box
        p1_hitbox= self._get_hitbox(player=self.player_1)
        p2_hitbox= self._get_hitbox(player=self.player_2)

        #get hurt box
        p1_hurtbox= self._get_hurtbox(player=self.player_1)
        p2_hurtbox = self._get_hurtbox(player=self.player_2)

        dmg_dealt = 0
        dmg_taken = 0

        if p2_hitbox is not None and self._aabb(p2_hitbox, p1_hurtbox):
            dmg_dealt = self.get_attack_damage(player=self.player_2)
            self.player_1.health -= dmg_dealt

        if p1_hitbox is not None and self._aabb(p1_hitbox, p2_hurtbox):
            dmg_taken = self.get_attack_damage(player=self.player_1)
            self.player_2.health -= dmg_taken

        return dmg_dealt, dmg_taken

    @staticmethod
    def _tick_anim(player:Player):
        player.current_frame +=1

        if player.current_frame>=ANIM_LEN[player.current_state]:
            player.current_frame=0
            if player.current_state not in LOOPING_STATES:
                player.current_state=0
                player.locked=False

    @staticmethod
    def _move(player:Player,faces_right):
        if player.current_state == 2 and player.current_frame == 2:
            player.x= min(110,player.x+1) if faces_right else max(4, player.x-1)
        elif player.current_state == 1 and player.current_frame == 0:
            player.x= max(4, player.x-1) if faces_right else min(110, player.x+1)

    def _p1_heuristic(self):

        dist = self.player_2.x - self.player_1.x

        if dist > 20:
            return 2
        elif dist <8:
            return 1

        return self.np_random.choice([3,4,5],p=[0.5, 0.3, 0.2])