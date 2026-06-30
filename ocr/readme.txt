Please ensure "ocr resource" folder has been prepared as descripted in Developer Guide document.

Platform: Windows
Now you can try ocr demo in Foxit PDF SDK:
1. Open ocr project in Visual Studio.
2. Set the local path of "ocr reousce" to argument ocr_resource_path.
3. Build and run ocr demo.
Generated files can be found in \simple_demo\output_files\ocr.

Platform: Linux  64bit
You can refer to the /examples/simple_demo/readme.md file.
1. cd examples/simple_demo 
2. cmake -DPRJ_NAME=ocr
3. make
4. ./ocr_linux64
Generated files can be found in \simple_demo\output_files\ocr.