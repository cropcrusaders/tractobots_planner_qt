# 🚜 Tractobots Planner Qt

A CNC‑style, C++/Qt desktop application for planning and exporting **pass‑by‑pass** field‑robot paths.  
Draw your field boundary, specify row spacing, headland, turn radius and backup distance, then **export ready‑to‑run G‑code** with embedded heading commands!

---

## ✨ Features

- **Boundary‑to‑Rows**  
  Automatically generates parallel “row” polylines inside any polygonal field with a user‑specified headland buffer.

- **Zig‑Zag & Reverse**  
  Optionally alternate driving direction on each pass.  
  Add a custom backup distance at each end of row before turning.

- **Smooth Turns**  
  Insert semicircular arcs of configurable radius between rows for gentle U‑turns (G2/CW arc commands).

- **G‑Code Export**  
  • `M3 S0/S1` for implement up/down  
  • `G0` for rapid moves with `; heading=…` comments  
  • `G1` for linear cuts or reverse moves  
  • `G2` for CW arcs (turn radius)  
  Outputs a single `.gcode` file ready for your robot or CNC‑style interpreter.

- **Interactive GUI**  
  • Click to define boundary points in the canvas  
  • Live preview of passes (blue) and turns (green)  
  • Adjustable parameters (spacing, headland, radius, backup, zig‑zag)  

---

## 📦 Prerequisites

- **Qt 6** (Widgets)  
- **Boost.Geometry**  
- **CMake ≥ 3.10**  
- A C++17‑capable compiler (GCC 9+, Clang 9+, MSVC 2019+)  

---

## 🛠️ Build Instructions

```bash
# 1. Clone or copy this repo
git clone https://github.com/your-org/tractobots_planner_qt.git
cd tractobots_planner_qt

# 2. Create build directory
mkdir build && cd build

# 3. Configure with CMake
cmake ..

# 4. Build the executable
cmake --build . --parallel

# 5. Run
./tracto_planner_qt
```

---

## 🚀 Usage

1. **Draw your field**  
   Click on the blank canvas to add boundary vertices (in Cartesian coordinates).

2. **Set your parameters**  
   - **Row Spacing (m)** – distance between adjacent passes  
   - **Headland (m)** – inward buffer margin (no‑cut zone)  
   - **Turn Radius (m)** – U‑turn smoothing radius (set 0 for sharp corners)  
   - **Reverse Dist (m)** – how far to back up at each row end  
   - **Zig‑Zag** – toggle alternate driving direction  

3. **Generate**  
   Click **Generate** to overlay the computed passes (blue) and arcs (green).

4. **Export G‑Code**  
   Click **Export G‑Code…**, choose a filename, and save.  
   The resulting `.gcode` includes heading comments for each move, ready for your tractor’s G‑code executor.

---

## 📐 How It Works

- **Row Generation**  
  1. Buffers the input polygon by **–headland** using Boost.Geometry’s buffer strategy  
  2. Computes the minimum‑area rotated rectangle to determine optimal row orientation  
  3. Intersects the rotated field with vertical lines spaced by **row spacing**, then re‑rotates back  

- **Plan Composition**  
  1. Straight pass segment (with optional reverse backing)  
  2. Backup segment of length **reverse_dist** (if set)  
  3. Turn arc segment (semicircle of radius **turn_radius**)  
  4. Repeat for each row, zig‑zagging if requested  

- **G‑Code Export**  
  - Rapid moves (`G0`) to entry points, with `; heading=…` comments  
  - Linear moves (`G1`) for cutting or backing (prefix `G1 R ` for reverse)  
  - Arc moves (`G2`) for CW semicircles (turns)  
  - `M3 S0/S1` commands to raise/lower implements  

---

## 🔧 Integration

- **Standalone GUI**: Run `tracto_planner_qt` and export `.gcode`  
- **Library Use**: Include `PathPlanner.h/.cpp` in your own C++ or ROS2 project to programmatically generate plans.  

---

## 📝 License

This project is released under the **[GPL v3](LICENSE)**.

---

## 🤝 Contributing

1. Fork the repo  
2. Create a feature branch (`git checkout -b feat/my‑awesome‑feature`)  
3. Commit your changes (`git commit -am 'Add new feature'`)  
4. Push (`git push origin feat/my‑awesome‑feature`)  
5. Open a Pull Request

Happy farming automation! 🌱🚜  
