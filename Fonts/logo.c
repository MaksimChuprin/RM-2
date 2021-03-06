#include "defines.h"
/*******************************************************************************
* image
* filename: unknown
* name: Image
*
* preset name: Monochrome
* data block size: 8 bit(s), uint8_t
* RLE compression enabled: no
* conversion type: Monochrome, Diffuse Dither 128
* bits per pixel: 1
*
* preprocess:
*  main scan direction: top to bottom
*  line scan direction: backward
*  inverse: yes
*******************************************************************************/

/*
 typedef struct {
     const uint8_t *data;
     uint16_t width;
     uint16_t height;
     } tImage;
*/
#include <stdint.h>


static const uint8_t image_data_Image[520] = {
    0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0x81, 0xf0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x71, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xe0, 0x00, 0x00, 
    0x00, 0x03, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x1c, 0x00, 0x00, 
    0x00, 0x0c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x03, 0x00, 0x00, 
    0x00, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0xc0, 0x00, 
    0x00, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xfc, 0x30, 0x00, 
    0x01, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x1f, 0xff, 0xff, 0x88, 0x00, 
    0x02, 0xc0, 0x00, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x7f, 0xff, 0xff, 0xe4, 0x00, 
    0x0f, 0x3e, 0x00, 0x1f, 0xff, 0xff, 0xf8, 0x01, 0xff, 0xff, 0xff, 0xfb, 0x00, 
    0x17, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0, 0x06, 0x80, 
    0x2f, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xe0, 0x0f, 0xc7, 0xc7, 0x80, 0x01, 0x40, 
    0x3f, 0xff, 0xfc, 0x00, 0xf8, 0xe3, 0xc0, 0x1f, 0xc7, 0x86, 0x00, 0x78, 0xc0, 
    0x5f, 0xff, 0xfe, 0x00, 0xf8, 0xc3, 0x80, 0x1f, 0xc7, 0x04, 0x03, 0xff, 0x20, 
    0xbf, 0xff, 0xff, 0x00, 0x78, 0xcf, 0x80, 0x3f, 0x87, 0x04, 0x07, 0xff, 0x90, 
    0xbf, 0xff, 0xff, 0x00, 0x30, 0x8f, 0x80, 0x3f, 0x8e, 0x08, 0x0f, 0xff, 0xd0, 
    0xff, 0xff, 0xff, 0x80, 0x30, 0x1f, 0x00, 0x3f, 0x8c, 0x08, 0x0f, 0xff, 0xd0, 
    0xff, 0xff, 0x00, 0x00, 0x30, 0x7f, 0x00, 0x3f, 0x88, 0x88, 0x1f, 0xff, 0xf0, 
    0xff, 0xff, 0xff, 0x80, 0x30, 0x3f, 0x00, 0x3f, 0x81, 0x88, 0x1f, 0xff, 0xd0, 
    0xff, 0xff, 0xff, 0x80, 0x31, 0x1f, 0x80, 0x3f, 0x83, 0x88, 0x0f, 0xff, 0xd0, 
    0xbf, 0xff, 0xff, 0x00, 0x31, 0x1f, 0x80, 0x3f, 0x83, 0x88, 0x0f, 0xff, 0x90, 
    0xbf, 0xff, 0xff, 0x00, 0x71, 0x8f, 0x80, 0x1f, 0x07, 0x1c, 0x07, 0xff, 0x10, 
    0x5f, 0xff, 0xfe, 0x00, 0xe3, 0x8f, 0xc0, 0x1f, 0x0f, 0x1e, 0x01, 0xfe, 0xa0, 
    0x2f, 0xff, 0xfc, 0x01, 0xe3, 0x8f, 0xc0, 0x0f, 0x1f, 0x1f, 0x00, 0x01, 0x40, 
    0x27, 0xff, 0xf8, 0x03, 0xe3, 0x8f, 0xe0, 0x07, 0xff, 0xff, 0xc0, 0x06, 0x40, 
    0x11, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xf0, 0x01, 0xff, 0xff, 0xff, 0xf8, 0x80, 
    0x0d, 0x9e, 0x00, 0x1f, 0xff, 0xff, 0xf8, 0x00, 0x7f, 0xff, 0xff, 0xe3, 0x00, 
    0x02, 0x60, 0x01, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x1f, 0xff, 0xff, 0x84, 0x00, 
    0x01, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x03, 0xff, 0xfc, 0x08, 0x00, 
    0x00, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x30, 0x00, 
    0x00, 0x31, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0xc0, 0x00, 
    0x00, 0x0c, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x03, 0x00, 0x00, 
    0x00, 0x03, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9c, 0x00, 0x00, 
    0x00, 0x00, 0x70, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 
    0x00, 0x00, 0x0f, 0x07, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x0f, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0xf8, 0x07, 0xff, 0xff, 0xfe, 0x01, 0xf0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00
};
const tImage Logo = { image_data_Image, 100, 40};
