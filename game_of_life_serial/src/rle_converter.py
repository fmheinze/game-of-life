import re
import sys
import numpy as np;

def rleToString(i_filename):
  l_string = ""
  l_found_start = False
  with open(i_filename, "r") as l_file:
    for l_line in l_file:
      if(l_found_start):
        l_string += l_line
      else:
        if(l_line[0] != "#"):
          l_found_start = True
  return l_string
      

def rleToArray(i_string, i_array, i_offset_x, i_offset_y):
  l_lines = i_string.split("$")
  l_y = i_offset_y
  for l_line in l_lines:
    l_split = re.findall(r'(\d*)(o|b)', l_line)
    l_x = i_offset_x
    for l_s in l_split:
      l_size = 1
      if(l_s[0]):
        l_size = int(l_s[0])
      l_symbol = l_s[1] == "o"
      for i in range(l_size):
        i_array[l_y, l_x] = l_symbol
        l_x+= 1
    l_y += 1
  
def writeArrayToPPM(i_filename, i_array, i_size_x, i_size_y):
  with open(i_filename, "w") as l_file:
    l_file.write("P5\n")
    l_file.write(str(i_size_x)+"\n")
    l_file.write(str(i_size_y)+"\n")
    l_file.write("255\n")
  with open(i_filename, "ab") as l_file:
    for l_y in range(i_size_y):
      for l_x in range(i_size_x):
        if(i_array[l_y, l_x] == 0):
          binary_data = b"\x00"
        else:
          binary_data = b"\xff"
        l_file.write(binary_data)
    
if __name__ == '__main__':
  if(len(sys.argv) != 6):
    print( "usage: python rle_converter size_image_x size_image_y pos_x pos_y filename" )
    print( "--------------------------------------------------------------------------" )
    print( "size_image_x: size of resulting ppm image in x direction" )
    print( "size_image_y: size of resulting ppm image in y direction" )
    print( "pos_x:        x position of converted rle figure in ppm image" )
    print( "pos_y:        y position of converted rle figure in ppm image" )
    print( "filename:     name of the rle file" )
    exit()
  
  #read arguments
  l_size_x = int(sys.argv[1])
  l_size_y = int(sys.argv[2])
  l_pos_x = int(sys.argv[3])
  l_pos_y = int(sys.argv[4])
  l_filename = sys.argv[5]

  #convert to string
  l_string = rleToString( l_filename )
   
  #convert to array
  l_array = np.zeros((l_size_y, l_size_x))
  rleToArray(l_string, l_array, l_pos_x, l_pos_y)

  #write to ppm
  l_filename_new = l_filename[:-3]+"ppm"
  writeArrayToPPM(l_filename_new, l_array, l_size_x, l_size_y)

