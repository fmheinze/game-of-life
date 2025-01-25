# Game of Life
Serial and MPI parallelized version of Conway's Game of Life.

## Required software
For running the serial version of Game of Life you need a C++ compiler, Python (in case you want to use .rle files) and ffmpeg (in case you want to save movies). For running the parallel version you need an MPI implementation like OpenMPI.

## How to use (serial version)
1. Navigate to the game_of_life_serial directory and run ```make``` in the terminal to compile the code using the Makefile (uses g++). The game_of_life executable will be created in the directory.
2. In order to evolve a state you have to provide an file that contains the initial state. You can run the program with two different types of initial state files, .rle or .ppm files. Some example files are provided in the initial_states directory.
   
3. Running with a .ppm initial state file:
   ```
   /path/to/game_of_life /path/to/initial/state.ppm <num_iterations> <save_movie>
   ```
   ```<num_iterations>``` is the number of time steps until the evolution stops.\
   ```<save_movie>``` can be either 0 (no) or 1 (yes), if you don't have ffmpeg set it to zero.
   
   The size of the domain is automatically obtained for the size of the .ppm image.
   
   For example, when being in the game_of_life_serial directory:
   ```
   ./game_of_life ./initial_states/spacefiller_state.ppm 300 1
   ```
5. Running with a .rle initial state file:
   ```
   /path/to/game_of_life /path/to/initial/state.rle <num_iterations> <save_movie> <width> <height> <pos_x> <pos_y>
   ```
   ```<num_iterations>``` is the number of time steps until the evolution stops.\
   ```<save_movie>``` can be either 0 (no) or 1 (yes), if you don't have ffmpeg set it to zero.\
   ```<width>``` is the width of the domain.\
   ```<height>``` is the height of the domain.\
   ```<pos_x>``` is the x-coordinate where the .rle file pattern will be placed.\
   ```<pos_y>``` is the y-coordinate where the .rle file pattern will be placed.
   
   For example, when being in the game_of_life_serial directory:
   ```
   ./game_of_life ./initial_states/spacefiller2.rle 500 1 200 400 90 180
   ```
6. THe outputs will be saved in the output directory. In case you do not want to save a movie file only the .ppm images of the individual iterations are saved.

## How to use (parallel version)
1. Navigate to the game_of_life_parallel directory and run ```make``` in the terminal to compile the code using the Makefile (uses mpicxx). The game_of_life executable will be created in the directory.
2. Running the program works similar as for the serial version but use the ```mpirun -n <num_procs>``` command where you specify the number of processes that you want to use, e.g.
   ```
   mpirun -n 4 ./game_of_life ./initial_states/spacefiller2.rle 500 1 200 400 90 180
   ```

   
   
