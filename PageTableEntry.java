import java.io.File; 
import java.util.Scanner; 
import java.io.FileNotFoundException; 
import java.util.LinkedList;
import java.util.*;






////////////////////////////////////////      PAGE TABLE ENTRY      ////////////////////////////////////////////////

/*

page table entry tracking pages, flags fot written to or not, 
page number for specific table entry 
Linkef list of accesses specfically for opt algorithm 
last line processed or accessed so track flow for table entry ---opt 


*/


public class PageTableEntry{

    int last; //int of last line accessed 
    int flag; //an int 0 or 1 indicating if the page has been written to
    long page; //page number 
    LinkedList<Integer> access; //list of mem accesses to cer page for OPT 


    public PageTableEntry(int x, long y){//opt construct
        
        last=x;
        flag=0; //false
        page=y;
        access=new LinkedList<Integer>(); //only for opt 

    }

    public PageTableEntry(long y){//lru construct 

        page=y;
        flag=0; //false 
        
    }

    public String toString(){
        return String.valueOf(page);
    }
}

