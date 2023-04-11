# Extensometer
A simple program for tracking dots on a tensile test specimen. 

**Usage:**



On the command line:

```bash
AJA_extenso.exe <reference image.png> 
```

Note the assumption is windows style paths. 

Alternatively:

**Drag the reference image onto the AJA_extenso.exe file. This is equivalent to the command line usage.**



The remainder of the images are assumed to be in the same folder as the reference image with filenames enumerated with the order in which they were taken. Therefore, the reference image should be named 1.png (or any other appropriate file extension).

*RawData.csv* is assumed to have the remainder of the data and *ImageMapping.csv* is assumed to have the time stamp for each image. 

Use the left mouse button and drag to select the first area of interest. Use the right mouse button similarly to select the second area of interest. Click the middle mouse button when ready. The analysis will be shown in the console. Close the image window when done. 

Data will be output in *DICResults.csv* and *Results_sync.csv*. With the raw data and data synchronized with force measurements respectively. 



 

