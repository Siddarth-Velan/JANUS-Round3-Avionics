Q1)
setup system states corresponding to each possible point in journey
default state = idle for starting purposes,
make cases for each state for conversion to the next state, setting up idle to ascent first
make it such that after payload is deployed, idle state counts as landing
to be very honest, quite less experience with arduino(basically only the first induction task), so apart from the state logic, most of cpp syntax is ai generated
thoroughly read through and checked code flow but function syntaxes still are unfamiliar territory 
Q2)
setup data transmission stream:
  i)taking raw data
  ii)decoding it into understandable language
created empty arrays for data in x and y axes
x axis is set to update with time, y is from arduino data
append the realtime data into the originally empty array and take the last element, updating it to the graph, rather than reloading the entire thing
-brownie)
  most of the original structure is followed
  changes include:
    i) adding extra axes
    ii) changing plot type to 3d to enable full traj viewing
    iii) data was in $gngga format and hence had to first convert the data to normal decimal coords for lat long and alt
    iv) conversion formula straight from google
    v) made south and west negative on their respective axes
    
