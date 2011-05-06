/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: YUV422toUYVY.cpp,v 1.2 2008/05/27 01:29:56 asuraparaju Exp $ $Name: Dirac_1_0_2 $
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
* the specific language governing rights and limitations under the License.
*
* The Original Code is BBC Research and Development code.
*
* The Initial Developer of the Original Code is the British Broadcasting
* Corporation.
* Portions created by the Initial Developer are Copyright (C) 2008.
* All Rights Reserved.
*
* Contributor(s):
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU General Public License Version 2 (the "GPL"), or the GNU Lesser
* Public License Version 2.1 (the "LGPL"), in which case the provisions of
* the GPL or the LGPL are applicable instead of those above. If you wish to
* allow use of your version of this file only under the terms of the either
* the GPL or LGPL and not to allow others to use your version of this file
* under the MPL, indicate your decision by deleting the provisions above
* and replace them with the notice and other provisions required by the GPL
* or LGPL. If you do not delete the provisions above, a recipient may use
* your version of this file under the terms of any one of the MPL, the GPL
* or the LGPL.
* ***** END LICENSE BLOCK ***** */

/*****************************************************************
File UYVYtoYUV422.cpp

Utility for converting a sequence of frames, stored in a single
file in raw planar YUV422 format, to a single output file in which they are
stored in raw UYVY format.
This utility is a filter taking input on stdin and generating its
output on stdout.
UYVY format stores the 422 YUV image with the U/V components
interleaved with the Y components. It is a sequence of sets of 4 bytes
containing U, Y, V, Y components of two horizontally adjacent pixels.
The U/V components are co-sited with the first luminance sample. In 422 YUV
format the U and V colour components are subsampled 2:1 horizontally.

Original author: Thomas Davies (based on filters by Tim Borer)
****************************************************************/

#include <stdlib.h> //Contains EXIT_SUCCESS, EXIT_FAILURE
#include <iostream> //For cin, cout, cerr
#include <algorithm> //For fill_n
using std::cout;
using std::cin;
using std::cerr;
using std::clog;
using std::endl;
using std::ios_base;
using std::fill_n;

#include "setstdiomode.h"
using namespace dirac_vu;

int main(int argc, char * argv[] ) {

    if (argc != 4) {
        cout << "\"YUV422toUYVY\" command line format is:" << endl;
        cout << "    Argument 1: width (pixels) e.g. 720" << endl;
        cout << "    Argument 2: height (lines) e.g. 576" << endl;
        cout << "    Argument 3: number of frames e.g. 3" << endl;
        cout << "    Example: YUV422toUYVY <foo >bar 720 576 3" << endl;
        cout << "        converts 3 frames, of 720x576 pixels, from file foo to file bar" << endl;
        return EXIT_SUCCESS; }

    //Get command line arguments
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int frames = atoi(argv[3]);

    //Set standard input and standard output to binary mode.
    //Only relevant for Windows (*nix is always binary)
    if ( setstdinmode(std::ios_base::binary) == -1 ) {
        cerr << "Error: could not set standard input to binary mode" << endl;
        return EXIT_FAILURE; }
    if ( setstdoutmode(std::ios_base::binary) == -1 ) {
        cerr << "Error: could not set standard output to binary mode" << endl;
        return EXIT_FAILURE; }

    //Allocate memory for input and output buffers.
    const int YBufferSize = height*width;
    unsigned char *YBuffer = new unsigned char[YBufferSize];

    const int UVBufferSize = height*width/2;
    unsigned char *UBuffer = new unsigned char[UVBufferSize];
    unsigned char *VBuffer = new unsigned char[UVBufferSize];

    const int YUVBufferSize = height*width*2;
    unsigned char *YUVBuffer = new unsigned char[YUVBufferSize];

    //Create references for input and output stream buffers.
    //IO is via stream buffers for efficiency
    std::streambuf& inbuf = *(cin.rdbuf());
    std::streambuf& outbuf = *(cout.rdbuf());

    for (int frame=0; frame<frames; ++frame) {

        clog << "Processing frame " << (frame+1) << "\r";

        //Read frame of Y
        if ( inbuf.sgetn(reinterpret_cast<char*>(YBuffer), YBufferSize) < YBufferSize ) {
            cerr << "Error: failed to read frame " << frame << endl;
            return EXIT_FAILURE; }

        if ( inbuf.sgetn(reinterpret_cast<char*>(UBuffer), UVBufferSize) < UVBufferSize ) {
            cerr << "Error: failed to read frame " << frame << endl;
            return EXIT_FAILURE; }

        if ( inbuf.sgetn(reinterpret_cast<char*>(VBuffer), UVBufferSize) < UVBufferSize ) {
            cerr << "Error: failed to read frame " << frame << endl;
            return EXIT_FAILURE; }

        // Write into a single buffer
        int YUVpos = 0;
        for (int uvpos=0,ypos=0; uvpos<UVBufferSize; ) {
            YUVBuffer[YUVpos++] = UBuffer[uvpos];
        YUVBuffer[YUVpos++] = YBuffer[ypos++];
        YUVBuffer[YUVpos++] = VBuffer[uvpos++];
        YUVBuffer[YUVpos++]=YBuffer[ypos++];
        }

            
        //Write frames of YUV
        if ( outbuf.sputn(reinterpret_cast<char*>(YUVBuffer), YUVBufferSize) < YUVBufferSize ) {
            cerr << "Error: failed to write frame " << frame << endl;
            return EXIT_FAILURE; }
    }

    delete [] YUVBuffer;
    delete [] YBuffer;
    delete [] UBuffer;
    delete [] VBuffer;

    return EXIT_SUCCESS;
}
