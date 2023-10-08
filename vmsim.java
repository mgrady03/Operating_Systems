import java.io.File; 
import java.util.Scanner; 
import java.io.FileNotFoundException; 
import java.util.LinkedList;
import java.util.*;

/*

vmsim first decalre and initlaize vairbales as needed; 
lru or opt for algorithm---later on check for which one else reject invalid 
page table size and number of frames important for transfer to simulation 
ratio takes into account the amount of frames per process for memory 

followed by file variables scanner to read, file for tracefile, and name of file 

offset for page hex value...README help

current line we are on of file,page, etc 

    END OF VARIABLE LIST?

check arg length===9 

set variables equal 
try, catch 
filename, tracefile, scanner 
try, catch 
print---look at readme for exact format 


simulate lru or opt 


*/

////////////////////////////////////////////////     VMSIM      /////////////////////////////////////////////////////////////

/* 
This is the virtual machine part of the project. after the initialization of some major vairbales, 
we can move into try/catch to make sure we have not only the right file, but also the right length of args 
once we check these, we can see if we need to simulate LRU or OPT. 

We can create a simulation object to use so that when we pull out 'lru' or 'opt' we can simply use that simulation object.LRU or .OPT


at the end we can print out our simulation w stats---> these stats have been passed thorugh vmsim to the specific simulation for accurate nums


*/

public class vmsim{

    public static void main(String [] args) throws FileNotFoundException{

        String alg=null; //lru or opt type 
        int pSize=0; //pageTable size 
        int frames=0; //number of frames 

        //Hashmap maybe or arrayList?
        String [] ratio=null; //how many frames per easy process 
        int x=0;//ratio send thru lru and opt 
        int y=0;//^^^


        String filename= null; //trace file usage!! filename obv 
        File tracefile=null; //trace file, the ac tracefile 
        Scanner file_reader=null;//tracefile

        int pageOffset=0; //hex for offset of page #-->README 
        int curr_line=0; //current line of the file 

//END OF VARIABLE LIST?

//first check for length 
/*
        if(args.length>9 || args.length<9){ 
            System.out.println("Argument length not valid.");
            System.exit(8);
        }

*/   
        if(args.length!=9){ //we need certain length to run, if not, error n exit
            System.out.println("Argument length not valid.");
            System.exit(8);
        }


        //try to read info from args, if not catch error!

        //    ./vmsim -a <opt|lru> –n <numframes> -p <pagesize in KB> -s <memory split> <tracefile>
        //^readme for numbers

        //Access Type: A single character indicating whether the access is a load ('l') or a store ('s'). The ‘s’ mode modifies the address and sets the dirty bit to true.
        //Address: A 32-bit integer (in unsigned hexadecimal format) specifying the memory address that is being accessed. For example, "0xff32e100" specifies that memory address 4281524480 (in decimal) is accessed.
        //Process: Either 0 or 1, representing which of the two processes made the memory access. Fields on the same line are separated by a single space. Example trace lines look like:

        try{
            alg=args[1];
            pSize=Integer.parseInt(args[5]);
            frames=Integer.parseInt(args[3]);
            
            ratio=args[7].split(":"); //split here for ratio to pull apart and hold 
            filename=args[8]; //filename is #8
            x=Integer.parseInt(ratio[0]);
            y=Integer.parseInt(ratio[1]);

            //pageOffset=(int)Math.floor((((Math.log(pSize)/Math.log(2)))+10));
            pageOffset=(int)Math.ceil((((Math.log(pSize)/Math.log(2)))+10));
        }
        catch(NumberFormatException nf){ //number not found catch 
            System.out.println("not valid.");
            System.exit(8);
        }


        //read info from file 
        try{
            tracefile=new File(filename);
            file_reader= new Scanner(tracefile);
        }
        catch(FileNotFoundException e){ //file not found excetion 
            System.out.println("Algorithm: "+alg);
            System.out.println("Number of frames: "+frames);
            System.out.println("Page size: "+pSize);
            System.out.println("Total memory accesses: "+ 0);
            System.out.println("Total page faults: "+0);
            System.out.println("Total writes to disk: "+0);

            System.exit(0);
            //System.exit(8);

        }

    Simulation simulation=null; //interface simulation + toString 
    //object that is holds a simulation-- it can either be lru or opt 

        if(alg.equals("opt")) //if opt is picked, we simulate opt
            simulation =new OPT(frames, pSize, x, y, tracefile, pageOffset);
        else if(alg.equals("lru"))//else if lru is picked, we simulate lru 
            simulation=new LRU(frames, pSize, x, y);
        else 
        {
            System.out.println("not valid"); //not an options, neither 
            System.exit(8);
        }


       

        char accessType=0;//access, load, or store 
        //A single character indicating whether the access is a load ('l') or a store ('s'). 
        //The ‘s’ mode modifies the address and sets the dirty bit to true.
        long pageNum=0; // which page number is being accessed 
        int process=0; // 0 or 1, process accessing memory 
        //Either 0 or 1, representing which of the two processes made the memory access. 
        //Fields on the same line are separated by a single space. Example trace lines look like:


        while(file_reader.hasNextLine()){ //run through file 

            String[] info= file_reader.nextLine().split(" ");//divide by spaces, same as python? hold in string info of line 

            try{

                accessType=info[0].charAt(0); //access type--pulling an s or l
                //The ‘s’ mode modifies the address and sets the dirty bit to true.

                pageNum=Long.decode(info[1]); //decode to long value of page number
                pageNum=pageNum >>> pageOffset; //caculate offset 0--- total # of offset 
                process=Integer.parseInt(info[2]);
            }
            catch(NumberFormatException nf){
                System.out.println("not valid");
                nf.printStackTrace();
                System.exit(8);
            }

            simulation.simulate(accessType, pageNum, process, curr_line);//check simulation class!
            curr_line++; //move onto next line 
        }
        System.out.println(simulation);//to string i simulation class!--opt, lru 
        }

}
