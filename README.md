
# Ramster's Revenge
Our game, Ramster’s Revenge, is named after the game’s protagonist: a genius hamster in a ball who the player will control. Ramster’s Revenge tells the story of the titular main character’s journey breaking out of the lab in which he was made. An evil corporation experimenting on highly intelligent hamsters is in for a rude awakening when Ramster, the smartest of the test subjects escapes from his cage! Shortly after breaking out of the cell, Ramster comes across the HamHook 3000, a high tech grappling hook that allows Ramster to swing across the map and aid in his escape. Living up to his name, Ramster also uses the HamHook to ram into enemies, such as evil lab workers trying to put Ramster back in his cell. Determined and armed, Ramster will stop at nothing to see the outside world and fulfill his dream of living as a normal housepet hamster. Finally, Ramster also has a special catchphrase for when he gets really serious: 
**"I’M GOING HAM!"**

![sprite_4](https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-20/assets/5330/5894aea9-ad8e-4f6a-b259-f5ee62c584f8)
[![Video Demo](https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-20/assets/5330/c758fe05-cd88-492c-8344-b37af1e687c9)](https://www.youtube.com/watch?v=6IpZmlqbe44)

https://www.youtube.com/watch?v=6IpZmlqbe44&feature=youtu.be

## Controls
| Input | Action |
|----------|----------|
| `A/D` | Move left/right |
| `Space` | Jump |
| `Left Click` | Attach/detach to a grapple point or surface|
| `W` | Retract grapple |
| `S` | Extend grapple |
| `Escape` | Open the pause menu |

## M4 Requirements
| Category | Task | Satisfying Feature(s) |
|----------|----------|----------|
| Playability | Sustain progressive, non-repetitive gameplay for at least 10 minutes | ✅ |
| Stability | Include fully completed and playable prior-milestone implementations, fix all bugs, consistent across displays, no crashing | ✅ |
| User Experience | Tutorial | ✅ |
| Robustness | Memory management, user input handling, realtime performance | ✅ |
| Reporting | Bug List, Test Plan, Video | See `docs` and submitted link |

## M4 Creative Elements
- [3, 11] Raycast grapple (grapple to any surface!)
- [1] Dynamically animated sprites (frame time based on velocity)
- [1] Animation overhaul (Ramster sprite is layered in glass ball, tilts based on velocity and does not rotate with the ball)
- [24] Hand drawn and animated Ramster assets, banners, buttons, etc; all designed to be coherent with the theme
- [27] Story elements with hand drawn story frames depicting Ramster's origin and fate
- [26] Audio overhaul (sound effects for many of Ramster's actions); music changes per level and there are sound effects that play accordingly with Ramster's actions
- [19] Leaderboard system (that is persisted and reloaded between runs of the game)
***

<details>
<summary>Expand for M3</summary>

## M3 Requirements
| Category | Task | Satisfying Feature(s) |
|----------|----------|----------|
| Playability | Sustain progressive, non-repetitive gameplay for at least 5 minutes | ✅ |
| Playability | During the 5 minutes, the player should be able to interact with the game and see new content for most of the time | ✅ |
| Robustness  | Memory management  | ✅  |
| Robustness | User Input | ✅ |
| Robustness | Realtime Performance | ✅ |
| Stability | Include fully completed and playable prior-milestone implementations | ✅ |
| Stability | Fix all bugs identified in prior marking sessions | ✅   |
| Stability | The game resolution and aspect ratio are consistent across different machines/displays | ✅ |
| Stability | The game code should support continuing execution and graceful termination, with no crashes, glitches, or other unpredictable behaviour | ✅ |
| Software Engineering | Updated test Plan | See `docs` |
| Reporting | Bug List | See `docs` |
| Reporting | Demo Video | See Canvas |

## M3 Creative Elements
- (Basic) Dynamic window resizing
- (Basic) Parallax
- (Advanced) Chain shape parsing and level loader

</details>

<details>
<summary>Expand for M2</summary>

## M2 Requirements
| Category | Task | Satisfying Feature(s) |
|----------|----------|----------|
| Improved Gameplay | Game AI Improvements | Obstacle, Walking, and Swarm Enemies |
| Improved Gameplay | Sprite Animations | Enemy Animations |
| Improved Gameplay | Improved Assets | Tile Textures |
| Improved Gameplay  | Mesh-based collision detection | Box2D Chain Shapes  |
| Improved Gameplay  | Gameplay tutorial | ✅ |
| Improved Gameplay  | FPS Counter | See Game Window Caption |
| Playability	 | 2-minutes of non-repetitive gameplay	| ✅ |
| Stability | Stable frame rate and minimal game lag | ✅ |
| Stability | No crashes, glitches, or unpredictable behaviour | ✅   |
| Software Engineering | Updated test Plan | See `docs` |
| Reporting | Bug List | See `docs` |
| Reporting | Demo Video | See Canvas |

## M2 Creative Elements
- (Basic) Textured tiles
- (Basic) Accurate physical interactions with the world
- (Advanced) Grapple improvements, including support for multiple grapples, and grapple retraction
- (Advanced) Enemy swarm behavior

</details>

<details>
<summary>Expand for M1</summary>
<p align="center">
  <img src="https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-20/assets/5330/eac1752b-a5cd-4048-8334-60de8acae6d4" width="600">
</p>
## M1 Basic Elements
| Category | Task | Satisfying Feature(s) |
|----------|----------|----------|
| Rendering | Textured Geometry | Ramster and Enemy Sprites   |
| Rendering | Basic 2D transformations   | Advanced Camera, Terrain Rendering, Ramster and Enemy Movement|
| Rendering | Key-frame/state interpolation | Advanced Camera   |
| Gameplay | Keyboard/mouse control | Player controllable with keyboard  |
| Gameplay | Random/coded action | Enemy entities track and move towards player   |
| Gameplay | Well-defined game-space boundaries | Walls keep entities enclosed within a playable room   |
| Gameplay | Simple collision detection & resolution | Player collision with terrain and enemy entities|
| Stability | Stable frame rate and minimal game lag | ✅ |
| Stability | No crashes, glitches, or unpredictable behaviour | ✅   |
| Software Engineering | Test Plan | See `docs` |
| Reporting | Bug List | See `docs` |
| Reporting | Demo Video | See Canvas |

## M1 Creative Elements (for grading)
**[11 | Physics]**: Complex physical interactions with the environment
- (Basic) Accurate ball physics including angular velocity, rotation, and friction
- (Basic) Collision support for curved terrain
- (Advanced) Working grapple implemented with joints

**[20 | Software Engineering]**: External integration
- (Basic) Box2D Library used in ball, enemies, grapple, slope, and world system


## M1 Extra Creative Elements (save for later)
**[3 | Graphics]**: Complex geometry
- Rendering curved terrain; specifically translate, scale, and rotate line segments into place to form a curve
- Advanced camera mechanics with lock-on delay, view borders, and grapple support; specifically the upgraded camera projection matrix and logic for conditional camera movement based on player physics like the Sonic franchise of games

## M1 Interpolation Implementation
Linear interpolation is implemented whenever the camera performs movement not centered on the player. The `lerp()` function provided in the grading rubric is utilized. As an example, whenever the player grapples to a grapple point, the camera will go center on it. To calculate the in-between camera frames, the `lerp()` function is used in first the `x` direction, then the `y` direction. The `t` parameter is represented with a shift variable starting at `0` and incrementing by `0.02` up to `1`.

</details>

## Building the Game
1) Open Visual Studio (2022 is preferred)
2) From the repository root, do `mkdir build`, `cd build`, `Cmake ..`
4) `cd build` and open `ramster.sln` in Visual Studio
5) Build the Ramster project
6) Playable `ramster.exe` is located in `build/Debug`

## Group Members
| Member | Ownership |
|----------|----------|
| `Andrew Yang` | Grapple, Leaderboard |
| `Davis Song` | Physics Integration, Graphics, Assets |
| `Luke Lu` | Enemy AI, UI Integration |
| `Ning Leng` | Ball Physics, Level Generator |
| `Zach Chernenko` | Advanced Camera, Level Design |

