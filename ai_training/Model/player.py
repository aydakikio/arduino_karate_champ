from dataclasses import dataclass,field

@dataclass
class Player:
    health: int = 100
    x: int = 0
    current_frame: int = 0
    current_state: int = 0
    locked: bool = False
    hit_landed: bool = False