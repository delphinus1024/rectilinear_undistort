# rectilinear_undistort

Undistort photo taken by rectilinear lens.

## Description

rectilinear_undistort is command line tool that remove distortion of photograph caused by rectilinear lens characteristics.
This program is based on lensprofile_creator_cameramodel.pdf document provided by Adobe. You can find this pdf by googling.
So you can use the same parameters of lens profile (lcp) used by Camera Raw or Photoshop LR.

Before

<img src="https://github.com/delphinus1024/rectilinear_undistort/sample.tif" style="width: 600px;"/>

After

<img src="https://github.com/delphinus1024/rectilinear_undistort/result.tif" style="width: 600px;"/>

## Features

- Remove rectilinear lens distortion.
- Lens profile parameters can be specified by arguments.

## Requirement

- OpenCV 3.0.0 or above is preferable.
- Checked with win7 32bit + msys2 + gcc

## Usage

$rectilinear_undistort.exe [params] image result

        -?, -h, --help, --usage (value:true)
                show help
        --FX, --fx (value:0.403752)
                FocalLengthX
        --FY, --fy (value:0.403752)
                FocalLengthY
        --IX, --ix (value:0.500000)
                ImageXCenter
        --IY, --iy (value:0.499867)
                ImageYCenter
        --R1, --r1 (value:-0.071356)
                RadialDistortParam1
        --R2, --r2 (value:0.022986)
                RadialDistortParam2
        --R3, --r3 (value:0.0)
                RadialDistortParam3
        -S, -s
                show image when finish
        --T1, --t1 (value:0.0)
                TangentialDistortParam1
        --T2, --t2 (value:0.0)
                TangentialDistortParam2

        image
                input image file
        result (default:result.tif)
                result image file

	
## Installation

1. Modify Makefile according to your OpenCV inludes and libs environment.
2. make

## Author

delphinus1024

## License

[MIT](https://raw.githubusercontent.com/delphinus1024/rectilinear_undistort/master/LICENSE.txt)

