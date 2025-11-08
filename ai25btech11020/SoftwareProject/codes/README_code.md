first have to convert the given figs i.e. einstein.jpg globe.jpg and greyscale.png images to pgm files using the code to_pgm.c 
by running the going to the path by creating a directory to where the code is present and run 

first run /codes/c_main/ gcc to_pgm.c -o f
then       ./f

that code will run and 3 pgm files will  be generated in input folder of figs 

then  run gcc main.c pgm_io.c svd_utils.c -o f
then      ./f

now the total code will run and 12 pgm files will be generated in output folder of figs and a table of frobenious error in the tables is generated

now to convert the 12 pgm files to png files 

run gcc tp_png.c -o f
    ./f 

now 12 png images will be generated in the output folder of the figs folder \

in the input folder of the figs i have give the txt file for the k values that i have clearly mentioned in the code of main.c 