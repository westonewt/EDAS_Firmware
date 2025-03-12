To download gtk and to be able to run the program, do the following:

1. Update the package list  (optional):
   sudo apt update

3. Install gtk3.0:
   sudo apt install libgtk-3-0 libgtk-3-dev

5. Verify the installation:
   pkg-config --modversion gtk+-3.0

To run the program, do the following:
1. Download this code onto a any location on your raspberry pi using any method and navigate to it uisng "cd"
2. Build the program:
   gcc -o gui gui.c `pkg-config --cflags --libs gtk+-3.0`
