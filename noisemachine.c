/* A C-Based Noise Machine

A C code which generates different colours of noise: white, pink and red

- By Varun Khatri */

#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_statistics_double.h>
#include <math.h>
#include <portsf.h>
#include <stdlib.h>
#include <stdio.h>

// Enums defined for command line argument
enum
{
    ARG_PROGNAME,
    ARG_OUTFILE,
    ARG_TYPE,
    ARG_DUR,
    ARG_SRATE,
    ARG_ARGC
};

// defining packed array for storing complex numbers generated post FFT
#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

int main (int argc, char* argv[])
{
    int alpha;
    int dur;
    PSF_PROPS outprops;
    int ofd = -1;
    psf_format outformat = PSF_FMT_UNKNOWN;
    long totalsamples;
    double *samples_dbl;
    double *samples_abs;
    float *samples;
    const gsl_rng_type * T;
    gsl_rng * r;
    char* colour[3] = {"White", "Pink", "Red"};

    printf ("\n*** C-Based Noise Machine ***\n"
      "A noise generator which generates clips of white, pink and red noise\n\n");

    // validates if the user has entered correct number of command line arguments
    if (argc != ARG_ARGC)
    {
        printf ("insufficient arguments\n"
          "usage: noisemachine outfile type dur srate\n"
          "where type =:\n"
          "       0 = white\n"
          "       1 = pink\n"
          "       2 = brown\n"
          "dur   = duration of outfile in seconds (max 10)\n"
          "srate = required sample rate of outfile\n\n");
        return 1;
    }

    // converts the noise colour type to the exponent factor for 1/f noise and validates the input
    // coloured noise is also called 1/f noise
    alpha = atoi (argv[ARG_TYPE]);
    if (alpha < 0 || alpha > 2)
    {
        printf ("Error! Please enter an integer value between 0 and 2\n\n");
        return 1;
    }

    // stores the duration of outfile as integer and validates the input
    dur = atoi (argv[ARG_DUR]);
    if (dur < 0 || dur > 30)
    {
        printf ("Error! Please enter an integer value between 0 and 10\n\n");
        return 1;
    }

    // initialising portsf
    if (psf_init())
    {
  		  printf ("Error! Unable to start portsf\n\n");
  		  return 1;
  	}

    // stores the sampling rate of outfile in outprops and validates the input
    outprops.srate = atoi (argv[ARG_SRATE]);
  	if (outprops.srate <= 0)
    {
  		  printf ("Error! Sampling rate must be positive\n\n");
  	    return 1;
  	}
    // updates other parameters of outprops. the outfile will be of IEEE float type and will have one channel
  	outprops.chans = 1;
  	outprops.samptype = PSF_SAMP_IEEE_FLOAT;
  	outprops.chformat = STDWAVE;

    // the file extension is extracted and updated in outprops
    outformat = psf_getFormatExt (argv[ARG_OUTFILE]);
  	if (outformat == PSF_FMT_UNKNOWN)
    {
  		  printf ("Error! Outfile name %s has unknown format.\n"
  			   "Use any of .wav, .aiff, .aif, .afc, .aifc\n\n", argv[ARG_OUTFILE]);
        return 1;
  	}
  	outprops.format = outformat;

    printf ("Generating %s Noise...\n\n", colour[alpha]);

    // calculating total number of samples in a file based on duration and sampling rate
    totalsamples = outprops.srate * dur;

    // allocating memory for sample values (and an array that stores their absolute values) based on total number of samples
    samples_dbl = (double *) malloc (totalsamples * sizeof(double));
    samples_abs = (double *) malloc (totalsamples * sizeof(double));

    // initialising the random number generator
    gsl_rng_env_setup();
    T = gsl_rng_default;
    r = gsl_rng_alloc (T);

    // this is the main part where we generate sequence to write to sound file
    if (alpha == 0)   // white noise
    {
        for (int i = 0; i < totalsamples; i++)
        {
            // generates random numbers with standard normal distribution
            samples_dbl[i] = gsl_ran_gaussian (r, 1); // stores the numbers here
            samples_abs[i] = fabs (samples_dbl[i]); // stores the absolute value
        }
    }

    else if (alpha == 2)    // red noise
    {
        // generates normally distributed random numbers with zero mean and standard deviation of (1 / sampling freq)
        samples_dbl[0] = gsl_ran_gaussian (r, sqrt(1.0 / outprops.srate));
        for (int i = 1; i < totalsamples; i++)
        {
            // since red noise is a random walk, each sample is a cumulative sum of current sample and all previous samples
            samples_dbl[i] = samples_dbl[i - 1] + gsl_ran_gaussian (r, sqrt(1.0 / outprops.srate));
            samples_abs[i] = fabs (samples_dbl[i]);
        }
    }

    else   // pink noise
    {
        // generating white noise
        for (int i = 0; i < totalsamples; i++)
        {
            samples_dbl[i] = gsl_ran_gaussian (r, 1);
        }

        // setting up FFT operations
        double fftdata[2 * totalsamples];
        gsl_fft_complex_wavetable * wavetable;
        gsl_fft_complex_workspace * workspace;

        // updating the packed array to store gaussian sequence in a + jb format
        for (int i = 0; i < totalsamples; i++)
        {
            REAL (fftdata, i) = samples_dbl[i];
            IMAG (fftdata, i) = 0.0;
        }

        // allocation memory for FFT computations
        wavetable = gsl_fft_complex_wavetable_alloc (totalsamples);
        workspace = gsl_fft_complex_workspace_alloc (totalsamples);

        // performing FFT to convert data into freq domain
        // the n point complex time domain sequence 'fftdata' is converted to n point FFT sequence
        gsl_fft_complex_forward (fftdata, 1, totalsamples, wavetable, workspace);

        int points;
        if (totalsamples % 2 != 0)
        {
            points = (totalsamples / 2) + 1;
        }
        else
        {
            points = totalsamples / 2;
        }

        // performing spectral processing where each DFT point is scaled by 1/f rule

        // scaling frequencies between 0 and pi
        for (int i = 0; i < points; i++)
        {
            REAL (fftdata, i) /= sqrt (i + 1);
            IMAG (fftdata, i) /= sqrt (i + 1);
        }

        // scaling frequencies between pi and 2pi
        for (int i = points; i < totalsamples; i++)
        {
            REAL (fftdata, i) /= sqrt (totalsamples - i);
            IMAG (fftdata, i) /= sqrt (totalsamples - i);
        }

        // inverting sequence back to time-domain
        gsl_fft_complex_inverse (fftdata, 1, totalsamples, wavetable, workspace);

        // freeing memory allocated for FFT operations
        gsl_fft_complex_wavetable_free (wavetable);
	      gsl_fft_complex_workspace_free (workspace);

        // the real data from IDFT is retrieved
        for (int i = 0; i < totalsamples; i++)
        {
            samples_dbl[i] = REAL(fftdata, i);
        }

        // converting to normal distribution
        double pink_mean = gsl_stats_mean (samples_dbl, 1, totalsamples);
        double pink_std = gsl_stats_sd (samples_dbl, 1, totalsamples);

        for (int i = 0; i < totalsamples; i++)
        {
            samples_dbl[i] = (samples_dbl[i] - pink_mean) / pink_std;
            samples_abs[i] = fabs (samples_dbl[i]);
        }
    }

    // now that sequences are generated, to write to audio files, we need to ensure there is no clipping
    // we do that by dividing all samples by max value
    double max = gsl_stats_max (samples_abs, 1, totalsamples);

    samples = (float *) malloc (totalsamples * sizeof(float));

    // samples are converted from double to float to write to a file
    for (int i = 0; i < totalsamples; i++)
    {
        samples[i] = (float) samples_dbl[i] / max;
    }

    // creates a sound file and writes sequence data data sample by sample
    ofd = psf_sndCreate (argv[ARG_OUTFILE], &outprops, 0, 0, PSF_CREATE_RDWR);
    if (ofd < 0)
		{
			  printf ("Error: unable to create outfile %s\n", argv[ARG_OUTFILE]);
    		goto exit;
		}

    for (int i = 0; i < totalsamples; i++)
    {
        if (psf_sndWriteFloatFrames (ofd, samples + i, 1) != 1)
        {
            printf("Error writing to outfile\n");
            break;
        }
    }

    printf ("%s Noise Generated!\n\n", colour[alpha]);

    // clears all dynamically allocated memory
    exit:
    	if (ofd >= 0)
    		if (psf_sndClose(ofd))
    			printf ("Error closing outfile %s\n", argv[ARG_OUTFILE]);
      if (samples_dbl)
          free (samples_dbl);
      if (samples_abs)
          free (samples_abs);
      if (samples)
          free (samples);
      gsl_rng_free (r);
      psf_finish();
}
