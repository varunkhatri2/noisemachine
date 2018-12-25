# A C-Bsaed Noise Machine
A C program that creates audio file with pink, white and red noise.
- by Varun Khatri

To be able to run the code on your computer, you need to have the GNU Scientific Library installed.

To install GSL, go to the directory 'gsl' in this same folder and follow the instructions
in the 'INSTALL' file. For MAC, simply run the command './configure && make && sudo make install' in Terminal. The working directory should be 'gsl'

Please email at vkhatri2@ur.rochester.edu

***************************************************************************************************************************

Sample run:

dhcp-10-5-2-149:C Code varunkhatri$ ./noisemachine

*** C-Based Noise Machine ***
A noise generator which generates clips of white, pink and red noise

insufficient arguments
usage: noisemachine outfile type dur srate
where type =:
       0 = white
       1 = pink
       2 = brown
dur   = duration of outfile in seconds (max 10)
srate = required sample rate of outfile

dhcp-10-5-2-149:C Code varunkhatri$ ./noisemachine white.wav 0 5 44100

*** C-Based Noise Machine ***
A noise generator which generates clips of white, pink and red noise

Generating White Noise...

White Noise Generated!

dhcp-10-5-2-149:C Code varunkhatri$ ./noisemachine brown.wav 2 5 44100

*** C-Based Noise Machine ***
A noise generator which generates clips of white, pink and red noise

Generating Red Noise...

Red Noise Generated!
