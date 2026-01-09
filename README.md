# Screen Shader

## Project Description
Screen Shader is a real-time screen post-processing application that applies various visual filters directly to your screen content. It allows users to enhance visuals, experiment with color grading, and apply artistic or accessibility-oriented effects on any application running on their desktop.

## Technologies Used
- **C++17** – Core application logic  
- **Windows API** – Screen capture, input handling, and overlay window management  
- **OpenGL** – Rendering and shader execution  
- **DirectX** – Retrieving information about the user's hardware, such as GPU name, disk space, RAM usage, and more  
- **GLSL** – Custom fragment and vertex shaders for real-time visual effects  
- **ImGui** – Interactive user interface for adjusting filters and settings  
- **JSON** – Saving and loading user profiles  

## Available Filters
- **Color Blindness Correction** – Adjust colors to improve visibility for red (protanopia), green (deuteranopia) and blue (tritanopia) color vision deficiencies
- **Color Blindness Simulation** – Simulate red (protanopia), green (deuteranopia) and blue (tritanopia) color vision deficiencies  
- **Screen Rotations** – Rotate screen by 90° left/right or 180°  
- **Screen Flips** – Horizontal and vertical screen flipping (visual only)  
- **Brightness** – Adjust overall lightness of the screen  
- **Gamma** – Apply gamma correction to modify mid-tone brightness  
- **Contrast** – Increase or decrease contrast levels  
- **Saturation** – Control the intensity of colors  
- **Red/Green/Blue Adjustment** – Independently scale RGB channels  
- **Reading Mode** – Apply softer, warmer colors to reduce eye strain during reading  
- **Color Inversion** – Invert all colors on the screen  
- **Black & White** – Convert screen content to grayscale  
- **Emboss** – Apply a 3D embossed effect to the screen content  
- **Vignette** – Darken screen edges with adjustable radius and smoothness  
- **Film Grain** – Add dynamic noise simulating film grain  
- **Sharpness** – Enhance details and edges on screen content  
- **Pixelate** – Apply pixelation with adjustable block size  
- **Kuwahara Filter** – Smooth areas while preserving edges for an artistic effect  
- **Blur** – Apply Gaussian blur with adjustable radius  
- **Difference of Gaussians (DoG)** – Edge enhancement / high-pass filter effect  

## Additional Features
- **Dark/Light Mode** – Switch ImGui UI between dark and light themes  
- **Collapse Overlay** – Reduce the overlay to a small title bar while keeping it on top of other windows  
- **PC Specifications** – View CPU usage, available RAM, GPU name, and other system information  
- **Profiles (Saves)** – Create, save, and load multiple shader configurations for different use cases  
- **Screenshots** – Capture the current screen content and save it as an image  

## Folder Structure
- `saves/` – Stores user profiles with individual shader settings  
- `screenshots/` – Stores captured screenshots  
- `shaders/` – GLSL shader files for all available filters  
