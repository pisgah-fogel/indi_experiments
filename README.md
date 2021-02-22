# indi_experiments
Indi experiments, home made telescope driver

# Setup
I use kstars / Ekos to control my Canon EOS 1100D camera. My plan is to control my home made mount with it

## List of Astronomy oriented projets I have
- 3D print a focuser (Crayford) - models available online

- Make a motorized AZ mount

- Test a new drive system for telescope: use a laser measuring technique to measure the motor (+gearbox) rotation. Coupled with an elastic connection with the telescope and thus filter small variations, resulting in a smother movement

# Platesolving

In order to have a local platesolving:

```
cd /tmp
wget https://github.com/dstndstn/astrometry.net/releases/download/0.82/astrometry.net-0.82.tar.gz
tar xvzf astrometry.net-0.82.tar.gz

cd astrometry.net-0.82
./configure

# Install python and cairo

sudo make install
sudo make install-indexes

# Get indexes
cd <Index directory>
wget -r http://broiler.astrometry.net/~dstn/4200/

# Edit config file with index directory

# Solve cf http://astrometry.net/doc/readme.html
solve-field demo/sdss.jpg
solve-field --scale-units arcsecperpix --scale-low 0.9423 --scale-high 1.1090 ...
# Canon EOS 60Da F800mm = 1.1090"/px - 1.1116"/px (1°35' wide)
# Canon EOS 70D F900mm = 0.9423"/px (58' wide)

# Use fits guess
solve-field --guess-scale --downsample 2 ...

# Tell where the image is within a radius
solve-field --ra, --dec, --radius

solve-field --scale-units arcsecperpix --scale-low 0.90 --scale-high 1.2 -d 1-10 --ra 10:50:21 --dec 10.5 --radius 15 /home/phileas/Pictures/orion_soir_3/Light/Light_60_secs_2021-02-20T21-51-18_038.fits



solve-field --scale-units arcsecperpix --guess-scale --downsample 2 -d 1-10 --ra 10:50:21 --dec 10.5 --radius 15 /home/phileas/Pictures/orion_soir_3/Light/Light_60_secs_2021-02-20T21-51-18_038.fits

# Works
# Returns
#log-odds ratio 14.2804 (1.59183e+06), 0 match, 0 conflict, 1 distractors, 33 index.
#RA,Dec = (59.2445,21.3313), pixel scale 0.105481 arcsec/pix.
solve-field --guess-scale --ra 3:48:43 --dec 24.3 --radius 5 test10_1.fits
```