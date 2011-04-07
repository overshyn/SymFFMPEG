/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: YUV422ItoYUV420I.cpp,v 1.1 2008/11/18 23:25:21 asuraparaju Exp $ $Name: Dirac_1_0_2 $
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
File YUV422ItoYUV420I.cpp

Utility for converting a sequence of frames, stored in a single
file in raw YUV422 format, to a single output file in which they are
stored in raw YUV420 format, by vertically filtering using a (1,3) or (3,1)
filter and subsampling taking into account the chroma sample positions
within each field.
This utility is a filter taking input on stdin and generating its
output on stdout.

This correctly handles interlaced i420

Original author: D Flynn (based on filters by Tim Borer/Thomas Davies)
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
        cout << "\"YUV422ItoYUV420I\" command line format is:" << endl;
        cout << "    Argument 1: width (pixels) e.g. 720" << endl;
        cout << "    Argument 2: height (lines) e.g. 576" << endl;
        cout << "    Argument 3: number of frames e.g. 3" << endl;
        cout << "    Example: YUV422ItoYUV420I <foo >bar 720 576 3" << endl;
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
    const int UVBufferSizeIn = height*width/2;
    unsigned char *UBufferIn = new unsigned char[UVBufferSizeIn];
    unsigned char *VBufferIn = new unsigned char[UVBufferSizeIn];
    const int UVBufferSizeOut = height*width/4;
    unsigned char *UBufferOut = new unsigned char[UVBufferSizeOut];
    unsigned char *VBufferOut = new unsigned char[UVBufferSizeOut];


    //Create references for input and output stream buffers.
    //IO is via stream buffers for efficiency
    std::streambuf& inbuf = *(cin.rdbuf());
    std::streambuf& outbuf = *(cout.rdbuf());

    for (int frame=0; frame<frames; ++frame) {

        clog << "Processing frame " << (frame+1) << "\r";

        //Read frames of Y then U then V components
        if ( inbuf.sgetn(reinterpret_cast<char*>(YBuffer), YBufferSize) < YBufferSize ) {
            cerr << "Error: failed to read Y component of frame " << frame << endl;
            return EXIT_FAILURE; }
        if ( inbuf.sgetn(reinterpret_cast<char*>(UBufferIn), UVBufferSizeIn) < UVBufferSizeIn ) {
            cerr << "Error: failed to read U component of frame " << frame << endl;
            return EXIT_FAILURE; }
        if ( inbuf.sgetn(reinterpret_cast<char*>(VBufferIn), UVBufferSizeIn) < UVBufferSizeIn ) {
            cerr << "Error: failed to read V component of frame " << frame << endl;
            return EXIT_FAILURE; }

        const int width2 = width/2;
        const int height2 = height/2;

        // middle lines
        for (int line_out=0,line_in=0; line_out<height2; line_out+=2, line_in+=4) {
            for (int x=0; x<width2; ++x ){
                /* topfield */
                UBufferOut[line_out*width2+x] = (3*UBufferIn[(line_in)*width2+x] + 1*UBufferIn[(line_in+2)*width2+x] +2) >> 2;
                VBufferOut[line_out*width2+x] = (3*VBufferIn[(line_in)*width2+x] + 1*VBufferIn[(line_in+2)*width2+x] +2) >> 2;
                /* botfield */
                UBufferOut[(line_out+1)*width2+x] = (1*UBufferIn[(line_in+1)*width2+x] + 3*UBufferIn[(line_in+1+2)*width2+x] +2) >> 2;
                VBufferOut[(line_out+1)*width2+x] = (1*VBufferIn[(line_in+1)*width2+x] + 3*VBufferIn[(line_in+1+2)*width2+x] +2) >> 2;
            }
        }

        //Write frames of YUV
        if ( outbuf.sputn(reinterpret_cast<char*>(YBuffer), YBufferSize) < YBufferSize ) {
            cerr << "Error: failed to write frame " << frame << endl;
            return EXIT_FAILURE; }
        if ( outbuf.sputn(reinterpret_cast<char*>(UBufferOut), UVBufferSizeOut) < UVBufferSizeOut ) {
            cerr << "Error: failed to write frame " << frame << endl;
            return EXIT_FAILURE; }
        if ( outbuf.sputn(reinterpret_cast<char*>(VBufferOut), UVBufferSizeOut) < UVBufferSizeOut ) {
            cerr << "Error: failed to write frame " << frame << endl;
            return EXIT_FAILURE; }
    }

    delete [] YBuffer;
    delete [] UBufferIn;
    delete [] VBufferIn;
    delete [] UBufferOut;
    delete [] VBufferOut;

    return EXIT_SUCCESS;
}
