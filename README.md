# README
This is the Client module of the Image Processing Application. This application communicates with the Server module to send an image or multiple images and filter/s to apply on it, and receive the image/s with the filter/s applied.

## Requirements
 - 64-bit operating system
 - OpenCV 4.8.0
 - Server companion module

## Configuring OpenCV 4.8.0
1. Download 4.8.0 from [OpenCV Releases page](https://opencv.org/releases/).
2. Install OpenCV to the desired location.
3. Add the OpenCV directory environment variable. Below are the steps to do this on Windows:
  - Open Start menu, search for 'Edit the system environment variables' and click on 'Open' as shown:

       <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/4122d972-7555-4a4b-838e-ea73aa59ac4f" width = "500">

  - In the 'System Properties' dialog box, click the 'Environment Variables...' button:

      <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/63b1ecd0-a510-4b7d-9d29-3a7a3698cd8f" width = "300">

  - Click 'New' at the bottom:
     
    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/6eb42f45-d1e2-4ca7-a112-0e1c035becd4" width = "300">

  - Type in 'OPENCV_DIR' (without the quotes) in the 'Variable name' field as shown, and click on the 'Browse Directory' button:

    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/9c0547cf-6da5-49b7-8978-cef459e4c40e" width = "500">

  - Browse to the 'build' folder of your OpenCV installation, as shown below. For example, if you installed OpenCV in the 'Documents' folder, browse to 'C:\Users\abc\Documents\opencv\build\'. Select the 'build' folder and click 'OK':

    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/be91ec41-fd1a-483a-853d-bc7b0bd88261" width = "300">

  - The 'Variable value' field in the 'New System Value' dialog box should now contain the address of the OpenCV build folder, as shown below. CLick OK:

    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/83e1754e-09b3-44f4-980b-52027040e978" width = "500">

  - The OPENCV_DIR variable will now be added to the 'System Variables' list in the 'Environment Variables' dialog box. Verify the value and click 'OK':

    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/8ab10cd7-a281-4659-8d5d-04b48dff141e" width = "300">

  - Finally, click OK in the 'System Properties' dialog box:

    <img src = "https://github.com/avnish32/APImageProcessorServer/assets/145987378/32eca162-82bc-4a26-b2a2-fdc57379cf5d" width = "300">

  - More information on how to set up environment variable for OpenCV can be found [here](https://www.opencv-srf.com/2017/11/install-opencv-with-visual-studio.html).

4. Build the application. Press Ctrl + B in Visual Studio for this.
5. Once the application is successfully built, the program is ready to execute.
   

## Preparing command-line arguments

The application takes user input in the form of command-line arguments, which should be in the following format:
```
<Server IP:Port> <Original image path> <Filter name> <Filter values>
```

The block 
```
<Original image path> <Filter name> <Filter values>
```
can be repeated any number of times depending on the number of images to submit to the server.

### Examples
```
127.0.0.1:8080 F:/MyImages/Hollywood/ClubOfFighting.jpg Flip Vertical
127.0.0.1:8080 F:/MyImages/Hollywood/AnIslandCalledShutter.jpg Rotate Clockwise 1 F:/MoreImages/Gaming/GTA6.jpg Resize 320 240
```



### Supported filters
Below is a list of the filters currently supported by the application and the parameters to send along with each filter:


1. **Resize** - Changes the size of the original image.

   Syntax:
   ```
   Resize <Target width> <Target height>
   ```

   Example:
   ```
   127.0.0.1:8080 F:/MyImages/Gaming/Catalyst.png Resize 800 600
   ```
   
   <img src="https://github.com/avnish32/APImageProcessorClient/assets/145987378/2fbd6d79-83ab-47ca-b23c-2fe235066bb3" width="800">


2. **Rotate** - Rotates the image in the specified direction, turning it the specified number of times.

   Syntax:
   ```
   Rotate <Rotation direction> <Number of turns>
   ```
   'Rotation direction' can be either 'Clockwise' or 'Anti-clockwise' (case-sensitive).
   'Number of turns' can be any positive whole number.

   Examples:
   ```
   127.0.0.1:8080 F:/MyImages/Gaming/Catalyst.png Rotate Anti-clockwise 2
   ```

   <img src="https://github.com/avnish32/APImageProcessorClient/assets/145987378/acaf147b-0e60-4387-aa7a-8feee2aa8795" width="800">

   ```
   127.0.0.1:8080 F:/MyImages/Gaming/Catalyst.png Rotate Clockwise 1
   ```
   
   <img src="https://github.com/avnish32/APImageProcessorClient/assets/145987378/7cb10d4c-b1c1-4758-8d85-55899bf7a3b8" width="800">



3. **Crop** - Crops the image according to the specified parameters.

   Syntax:
   ```
   Crop <X-coordinate of top left corner of crop area> <Y-coordinate of top left corner of crop area> <Width of crop area> <Height of crop area>
   ```

   Example:
   ```
   127.0.0.1:8080 F:/MyImages/Gaming/Catalyst.png Crop 350 100 200 500
   ```
   The above arguments will crop the image to an area 200 pixels wide and 500 pixels high, whose top left corner is at the point (350,100) with the origin (0,0) being on the top left corner of the original image.

   <img src="https://github.com/avnish32/APImageProcessorClient/assets/145987378/91cc803b-8c69-41ef-b338-1789b3c565c6" width="800">



4. **Flip** - Flips the image in the specified direction.

   Syntax:
   ```
   Flip <Flip direction>
   ```
   'Flip direction' can be 'Vertical' or 'Horizontal' (case-sensitive).

   Examples:
   ```
   127.0.0.1:8080 F:/MyImages/Gaming/Catalyst.png Flip Horizontal
   ```

   <img src="https://github.com/avnish32/APImageProcessorClient/assets/145987378/b3acacee-29f7-4a27-8fb6-621ea46055a2" width="800">


   ```
   127.0.0.1:8080 F:/MyImages/Gaming/Catalyst.png Flip Vertical
   ```

   <img src="https://github.com/avnish32/APImageProcessorClient/assets/145987378/bd58f838-3034-4d2f-8a40-f0a36da16db3" width="800">



5.  **Grayscale** - Converts the image to a grayscale image.

   Syntax:
   ```
   Grayscale
   ```

   Example:
   ```
   127.0.0.1:8080 F:/MyImages/Gaming/Catalyst.png Grayscale
   ```

  <img src="https://github.com/avnish32/APImageProcessorClient/assets/145987378/84d8ef12-46e6-49ad-b87c-1da4ecab9015" width="800">



6. **Brightness** - Changes the brightness of the original image.

   Syntax:
   ```
   Brightness <Brightness adjustment factor>
   ```
   'Brightness adjustment factor' can be any positive decimal value.
   A brightness adjustment factor of '0' gives a completely dark image, while '1' returns the same image. '2' gives an image having twice the brightness of the original.

   Example
   ```
   127.0.0.1:8080 F:/MyImages/Gaming/Catalyst.png Brightness 1.8
   ```

   <img src="https://github.com/avnish32/APImageProcessorClient/assets/145987378/85d873f0-1155-49a3-a2f7-8a363a87efbc" width="800">



### Points to note
- Image path should not contain any empty characters (whitespace, newline, return, tab etc.).
- Filter names (Resize, Grayscale etc.) are case-sensitive.
- Since the server runs on port 8080 of its respective machine by default, please modify the server URL accordingly.



## Running the application
1. Start the companion server module of the application. Please refer to the README file of the server for steps to start it up.
2. Start the client module of the application i.e. this module. Execution starts in APImageProcessorClient.cpp. Press Ctrl + F5 in Visual Studio to run the program. Take care to provide the appropriate command line arguments. 
3. On successful image processing, a console and two windows - the original and the modified images - should appear, as shown:


  <img src="https://github.com/avnish32/APImageProcessorClient/assets/145987378/9e63074c-b2bf-40ab-89fa-2aefe8186206" width="800">

