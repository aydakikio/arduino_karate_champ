import random
import numpy as np
import gymnasium as gym


env = gym.make('Taxi-v4')

alpha = 0.9 #how important the new information are | learning rate
gamma = 0.95 # how long term goals are important as mid term  goal
epsilon = 1.0 # all the actions are random
epsilon_decay = 0. #makes random action less during the time
min_epsilon = 0.01 #at leat should have 1% randomness
num_episodes = 10000 # how many time the agent will play the game
max_steps = 100

#5x5 grid -> 25 positions * 5 * 4 =500
q_table = np.zeros((env.observation_space.n, env.action_space.n))


def choose_action(state):
    if random.uniform(0,1)<epsilon:
        return env.action_space.sample()
    else:
        return np.argmax(q_table[state, : ]) #maximizes action base table


for episode in range(num_episodes):
    state, _ = env.reset()

    done = False

    for step in range(max_steps):
        action=choose_action(state)

        next_state, reward, done, truncated,info=env.step(action)

        old_value = q_table[state, action]
        next_max = np.max(q_table[next_state, :])

        q_table[state,action]=(1-alpha) * old_value + alpha * (reward+gamma * next_max)

        state = next_state

        if done or truncated:
            break


    epsilon = max(min_epsilon,min_epsilon*epsilon_decay)

env=gym.make('Taxi-v4', render_mode='human')

for episode in range(5):
    state , _ = env.reset()
    done = False

    print('Episode' , episode)

    for step in range(max_steps):
        env.render()
        action = np.argmax(q_table[state,:])

        next_state, reward, done, truncated,info=env.step(action)
        state = next_state

        if done or truncated:
            env.render()
            print('Finished episode', episode, 'with reward',reward)

            break

env.close()