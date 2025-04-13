# 🏃‍♂️ Olympic Runner 3D – OpenGL Game

**Olympic Runner** is a 3D Olympic-themed interactive game developed using C++ and OpenGL. This game was built as an individual project for the "DMET 502 – Computer Graphics" course at the German University in Cairo.

The player must navigate a sports arena filled with realistic 3D objects and animations to reach the finish line within a limited time while interacting with a dynamic environment and camera system.

---

## 🎯 Objective

Navigate the 3D Olympic field and reach the animated finish line before the timer runs out. If successful, the player wins the game. If the timer expires before reaching the goal, the game ends with a loss.

---

## 🛠️ Technologies Used

- **C++**
- **OpenGL**
- **GLUT** – for window management and rendering
- **SFML** – for background music and sound effects
- **Object-Oriented Programming** – for camera and vector abstractions

---

## 🎮 Game Features

- 🧍‍♂️ **Player Character**: Fully modeled runner with head, limbs, torso, shoes.
- 🧱 **3D Environment**:
  - Track with lane markings
  - Animated Olympic torch, podium, dumbbells, and flags
  - Finish line with poles and moving cylinder
- 🧠 **Game Logic**:
  - 40-second countdown timer
  - Game ends with a win or loss screen
  - Collision detection with walls and goal
- 🎥 **Camera Views**:
  - Top (`1`), Side (`2`), Front (`3`), and Reset (`4`)
  - Free camera control with WASD + QE keys
- 🎨 **Animations**:
  - Translate, rotate, scale, and color-change animations on key objects
  - Finish line poles and wall colors toggle automatically
- 🔊 **Audio**:
  - Background music
  - Collision and win/loss sound effects

---

## 🕹️ Controls

| Key        | Action                         |
|------------|--------------------------------|
| `↑ ↓ ← →`  | Move player                    |
| `1 - 4`    | Switch between camera views    |
| `H`        | Toggle animations              |
| `W/A/S/D`  | Move camera (free mode)        |
| `Q / E`    | Zoom camera in/out             |
| `Esc`      | Exit the game                  |

---

## 📁 File Structure

```bash
📁 OlympicRunner3D/
├── main.cpp              # Main source file with rendering, input, and logic
├── assets/               # Folder for sound files (e.g., gameWon.wav, collide.wav, etc.)
├── README.md             # This file
