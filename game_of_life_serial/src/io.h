#ifndef IO_H
#define IO_H

void parse_input_args(int argc, char** argv, std::string& filepath, int* num_iter, int* save_movie,
                      int* imax, int* jmax, int* pos_x, int* pos_y);
std::string generateFilename(int iter);
void save_state_ppm(unsigned char** grid, int width, int height, int iter);
void read_state_ppm(std::string filepath, unsigned char**& grid);
void read_state_rle(std::string filepath, unsigned char** grid, int size_x, int size_y, int pos_x, int pos_y);
void read_ppm_headerdata(std::string filepath, int* imax, int* jmax);
void save_mov(int width, int height);

#endif