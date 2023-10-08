
import java.io.File; 
import java.util.Scanner; 
import java.io.FileNotFoundException; 
import java.util.LinkedList;
import java.util.*;








////////////////////////////////////////         LRU SIMULATION       ///////////////////////////////////////////////////////////

/*
This is the LRU simulation. Least recently--- we have our basic variables to keep track of. 
we also have a helper class to maintain the maz size of entries as well as to keep track of RAM
and how many pages have been loaded into RAM from this process 

we use page table entry checks for flasg as written or not written to. in this case 1 or 0 
returns page needed

also the to String which is specific formating from README 

*/


public class LRU implements Simulation{

    int pSize; //page size 
    int frames; // number of frames 
    int access; // number of accesses to memory 
    int pFaults; //number of page faults
    int dWrites; //number of writes to disk 
    Helper_LRU[] process = new Helper_LRU[2]; //array holding processes currnetly in sim

    public LRU(int f, int p, int process1, int process2){

        frames=f;
        pSize=p;
        //number of frames we allocate to each process 
        process[0]=new Helper_LRU((f/(process1+process2))*process1); 
        process[1]=new Helper_LRU((f/(process1+process2))*process2);

        access=0;
        pFaults=0;
        dWrites=0;
    }

    public void simulate(char type, long page, int proc, int line){
        //first thing we have to do is increment memory access as we simulate through each step
        access+=1;
        Helper_LRU curr_process=process[proc]; //this hold the current process we r running
        PageTableEntry curr_page=getPage(curr_process, page); //is page in RAM?

        //lets check if not 
        if(curr_page==null){
            pFaults+=1;
            curr_page=new PageTableEntry(page);
            if(type=='s')//store
                curr_page.flag=1; //flag is true 
            if(curr_process.curr_loaded<curr_process.max){
                curr_process.mem_allocated.add(curr_page);//add page 
                curr_process.curr_loaded++; //current process loaded increment 
            }
            else{
                PageTableEntry pt= curr_process.mem_allocated.remove(0); //remove 
                if(pt.flag==1) //flagged as true 
                    dWrites+=1;//writes to disk incremented 
                curr_process.mem_allocated.add(curr_page); //add page 
            }
        }
        else{
            if(type=='s') //store 
                curr_page.flag=1; //flag it in page table file 
        }
    }

    public PageTableEntry getPage(Helper_LRU p, long page){

        //check for pages in ram 
        if(!p.mem_allocated.isEmpty()){
            for(int i=0; i<p.mem_allocated.size(); i++) //run through all to look for specific page 
            {
                PageTableEntry curr_page=p.mem_allocated.get(i); //get specific page 
                if(curr_page.page==page){
                    p.mem_allocated.add(p.mem_allocated.remove(i)); //when its referenced add to list 
                    return curr_page;
                }
            }
        }
        return null; //else return null 
    }

    /*

    SAME FOR LRU AND OPT 

    */ 
    public String toString(){
        return "Algorithm: LRU\n"+
        "Number of frames: "+frames+"\n"+
        "Page size: "+pSize+" KB\n"+
        "Total memory accesses: "+access+"\n"+
        "Total page faults: "+pFaults+"\n"+
        "Total writes to disk: "+dWrites;
        
    }

    private class Helper_LRU{ //this private class allows for the LRU process for refernce while using LRU
        long max; //RAM amt used by current process in running 
        LinkedList<PageTableEntry> mem_allocated; //the amount of mem allocated to said process in LL form 
        int curr_loaded; //num of pages loaded into ram by the current process 

        private Helper_LRU(int frame){
            max=frame;//max size is same as frame size 
            //initliaze
            mem_allocated= new LinkedList<PageTableEntry>(); 
            curr_loaded=0;
        }
    }

}

