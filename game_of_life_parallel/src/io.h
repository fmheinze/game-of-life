#ifndef IO_H
#define IO_H

void parse_input_args(int argc, char** argv, int rank, std::string& filepath, int* num_iter, int* save_movie,
                      int* imax, int* jmax, int* pos_x, int* pos_y);
std::string generateFilename(int iter);
void save_state_ppm(unsigned char** grid, int iter, int rank, int width, int height, int isize, int jsize, int ibegin, int jbegin);
void read_state_ppm(std::string filepath, unsigned char **grid, int rank, int imax, int isize, int jsize, int ibegin, int jbegin);
void read_state_rle(std::string filepath, unsigned char** grid, int rank, int imax, int jmax, int pos_x, int pos_y,
                    int isize, int jsize, int ibegin, int jbegin);
void read_ppm_headerdata(std::string filepath, int rank, int* width, int* height);
void save_mov(int width, int height);

#endif