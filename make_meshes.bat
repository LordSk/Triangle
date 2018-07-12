@echo off

echo player_ship
"ignore/geometryc.exe" -f "assets/player_ship.obj" -o "build/assets/player_ship.mesh" --ccw
"ignore/geometryc.exe" -f "assets/eye_enemy1.obj" -o "build/assets/eye_enemy1.mesh" --ccw