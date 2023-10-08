
import java.io.File; 
import java.util.Scanner; 
import java.io.FileNotFoundException; 
import java.util.LinkedList;
import java.util.*;






/////////////////////////////////   OPT SIMULATION   ////////////////////////////////////////////////////////

/*
this is the simulation for opt. optimal--same vairbales as lru and similar set up except we have to have a method 
getOPT that will get the optimal page to send back to sim for simulation to use 

toString is the same as LRU

Helper class is the same system-- instead we use Hash sets and hash maps 


        we used LL for process1 and 2 for LRU, 
        but we need something a little 
        more sophisticated for OPT--lets use a hashmap a
        nd fill it with the first pass 
        through of the trace file. 

keeping track of all info inside hashmap and hashset 

*/

public class OPT implements Simulation{

    int pSize; //page size 
    int frames; // number of frames 
    int access; // number of accesses to memory 
    int pFaults; //number of page faults
    int dWrites; //number of writes to disk 
    Helper_OPT[] process= new Helper_OPT[2];//array holding proccesses currnetly in sim

    @SuppressWarnings("unchecked")
    //@SuppressWarnings("unchecked")    
    public OPT(int f, int p, int process1, int process2, File file, int x)throws FileNotFoundException{


        frames=f; 
        pSize=p;
        Scanner file_reader = new Scanner(file);// scanner for file for trace initiliaziation 

        /*
        we used LL for process1 and 2 for LRU, 
        but we need something a little 
        more sophisticated for OPT--lets use a hashmap a
        nd fill it with the first pass 
        through of the trace file. 
        */

        HashMap<Long, PageTableEntry>[] p_memory= new HashMap[2]; //two processes, 2 spots 
        p_memory[0]= new HashMap<>(); //process 1 
        p_memory[1]= new HashMap<>(); //process 2 

        int curr_line=0; //current line in file while we read through 

        while(file_reader.hasNextLine()){
            
            String[] info= file_reader.nextLine().split(" "); //split w spaces--->the addy line
            int curr_process= Integer.parseInt(info[2]); //we r retriving only the page num 
            //^get current process from the info set split of the addy 
            HashMap<Long, PageTableEntry> curr= p_memory[curr_process];
            long page=Long.decode(info[1]); //same setup as vmsim class
            page=page>>x;// decode and offset 

            PageTableEntry page_number=curr.get(page); //try to get page number
            //if it exists, ok, if not we have to make a new entry intp page table 

            if(page_number==null){//doesnt exist 
                page_number= new PageTableEntry(curr_line, page);
                page_number.access.add(curr_line);
                curr.put(page, page_number);
            }
            else{ //does
            page_number.access.add(curr_line);

            }
            curr_line+=1;

        }


        //number of frames needed for each process 
        process[0]= new Helper_OPT(((f/(process1+process2))*process1), p_memory[0]);
        process[1]= new Helper_OPT(((f/(process1+process2))*process2), p_memory[1]);

        access=0;
        pFaults=0;
        dWrites=0;

        //^ same as LRU

    }

    public void simulate(char type, long page, int proc, int line){

        access+=1; //add 1 to the amount of time memory is accessed 
        Helper_OPT curr_process= process[proc];// current process 
        PageTableEntry curr_page=curr_process.ptable.get(page); //pnum
        curr_page.access.remove(0);

        if(type=='s')
            curr_page.flag=1;
        
        curr_page.last=line;

        if(!curr_process.mem_allocated.contains(page)){
            pFaults+=1; //page isnt in ram 

            if(curr_process.mem_allocated.size()==curr_process.max){ //equal to max size 
                Long rid = getOPT(curr_process); //what we want to get rid of 
                curr_process.mem_allocated.remove(rid); //remove it 
                
                if(curr_process.ptable.get(rid).flag==1){ //if the flag is true it is written to 
                    dWrites+=1; //increment disk writes bc its true 
                    curr_process.ptable.get(rid).flag=0; //set its write flag equal to 0 for false 
                }
            }
            curr_process.mem_allocated.add(curr_page.page); //add the page to ram 
        }
    }


        //get optimal page bc obviously we in OPT so we need this 
        //page tables as well---keep track 
    public Long getOPT(Helper_OPT optimal){ //opt optimal page find!!! 

        ArrayList<PageTableEntry> nope= new ArrayList<>(); //pages w no access in arraylist 
//i got carries away w the vairable names...
        int next=Integer.MIN_VALUE; //next access 
        Long best_rid=null; //best page to get rid of 

        for(Long l : optimal.mem_allocated){
                //THIS IS AN L NOT A 1 OMG SO MUCH DEBUGGING BLEH 

            PageTableEntry num = optimal.ptable.get(l); //value of optimal page tale entry 

            if(num.access.isEmpty()) //we empty 
                nope.add(num);
            else{
                if(num.access.get(0)> next){
                    best_rid=num.page;
                    next=num.access.get(0);
                }
            }
        }
      
        if(!nope.isEmpty()){
            int last = Integer.MAX_VALUE; //lru 
            for(PageTableEntry nap : nope){ //nap stoof for something like NAP but now i forget 
                if(nap.last<last){ //last of nap 
                    best_rid=nap.page; //we found the best page to get rid of 
                    last=nap.last;//re clarify 
                }
            }

        }

        return best_rid; //best one to get rid of lol
    }


    /*

    SAME FOR LRU AND OPT , yuh

    */ 

    public String toString(){ 
        return "Algorithm: OPT\n"+
        "Number of frames: "+frames+"\n"+
        "Page size: "+pSize+" KB\n"+
        "Total memory accesses: "+access+"\n"+
        "Total page faults: "+pFaults+"\n"+
        "Total writes to disk: "+dWrites;
        
    }

    private class Helper_OPT{ 

        HashSet<Long> mem_allocated; //basically ram but this is all about mem allocation
        HashMap<Long, PageTableEntry> ptable; //page table go tp page table entry class
        int max; //da max self explanatory 

        private Helper_OPT(int frames, HashMap<Long, PageTableEntry> hash){
            mem_allocated=new HashSet<>(); //stores in  hash but lowkey i wish i used a ll or array
            ptable=hash;  //get it hash lol its not 
            max=frames; //max num of frames 
        }
    }

}
