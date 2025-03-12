To download gtk and to be able to run the program, do the following:

1. Update the package list  (optional) /n
   **sudo apt update**

3. Install gtk3.0
   **sudo apt install libgtk-3-0 libgtk-3-dev**

5. Verify the installation
   **pkg-config --modversion gtk+-3.0**

To run the program, do the following:
1. Download this code onto a any location on your raspberry pi using any method and navigate to it
I downloaded it to the defualt folder "/home/edas", and named it "gui" you do not need to navigate to the default folder
Incase you downlaoded it to another folder, navigate to it using "cd (location)", in this case "cd /home/edas", Again navigation to the default folder is optional
2. Build the program
   **gcc -o gui gui.c `pkg-config --cflags --libs gtk+-3.0`**
   The first gui is the name of the executable file and the gui.c is the name of the gtk file, you can name them whatever you want, I chose to stick to gui and gui.c
