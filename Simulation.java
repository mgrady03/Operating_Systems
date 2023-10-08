
import java.io.File; 
import java.util.Scanner; 
import java.io.FileNotFoundException; 
import java.util.LinkedList;
import java.util.*;





//////////////////////////////////////////     SIMULATION INTERFACE    ////////////////////////////////////////////

/*
This simulation interface allows us to have an object that we can simulate opt or lru on. basically a object w the access type, address, 
process from vmsim (1/0), and the current line. These will get passed through as needed to opt or lru 



*/


    public interface Simulation {

    public String toString();


    public void simulate(char access, long addy, int a, int line);

    //from vmsim last 30 lines---> toString prints out simulation 
    //simualte uses access type or access, load, or store 
    //addy is the addess specific to memory to be accessed 
    //line is the line we are on 
    //a is the process that is accessing the memory so in this case same as vmsim 1 or 0
}



