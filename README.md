Can run only on 64-bit systems.

Needs OpenCV 4.8.0.
Download 4.8.0 from here: https://opencv.org/releases/

Set up environment variable: (Add screenshots)
Below tutorial for Windows:
Open start -> search for 'Edit the system environment variables'.
Click on 'Edit the system environment variables'.
In the 'System Properties' dialog box, click the 'Environment Variables...' button window as highlighted in the below image to open the 'Environment Variables' dialog box. 
Click 'New' at the bottom.
Variable name: OPENCV_DIR
Variable value: Address of the 'build' folder of your openCV installation. For example, if you installed openCV in the 'Documents' folder, value will be: C:\Users\abc\Documents\opencv\build\
Click OK.
Click OK again.

More info: https://www.opencv-srf.com/2017/11/install-opencv-with-visual-studio.html

Once OpenCV is set up, build the application. Can use Ctrl + B in Visual Studio.
After building, run the program. Execution starts in APImageProcessorClient.cpp
