/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: setstdiomode.h,v 1.3 2004/06/30 16:44:52 asuraparaju Exp $ $Name: Dirac_1_0_2 $
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
* Portions created by the Initial Developer are Copyright (C) 2004.
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
File setstdiomode.h

Utility for setting the mode of stdin/stdout and cin/cout to either
binary or text mode.

The function actually changes the mode of stdin/out but since these
use the same file id as cin/cout it changes the mode of those as well.

This function is only really relevant to Windows OS. *nixes use binary
IO mode all the time (there is no distinction beween binary and text mode).
The function does nothing under *nixes.

An argument is needed to control the mode. This is should be a
platform independent type. I have used std::ios_base::openmode for this
purpose. When a value of std::ios_base::binary is passed as a parameter
then the stdio and cin/out streams are set to binary mode (on Windows OS).

Return value: as _setmode function for Windows (-1 indicates error)
              0 for *nix (always succeeds)

Original author: Tim Borer
****************************************************************/

#ifndef dirac_utilities_setstdiomode
#define dirac_utilities_setstdiomode

#include <ios>      //Defines std::ios_base

namespace dirac_vu { //dirac video utilities namespace

    int setstdinmode(std::ios_base::openmode);
    int setstdoutmode(std::ios_base::openmode);

}  // end namespace dirac_vu

#endif // dirac_utilities_setstdiomode
