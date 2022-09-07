      /*************liu**************/
      PSCache *psc;
      /*************liu**************/
	  
	  /*****************liu******************/
Fault
Walker::WalkerState::startWalk()
{
    Fault fault = NoFault;
    assert(!started);
    started = true;
    setupWalk(req->getVaddr());
    if (timing) {
        nextState = state;
        state = Waiting;
        timingFault = NoFault;
        sendPackets();
    } else {
        do {
            walker->port.sendAtomic(read);
            PacketPtr write = NULL;
            fault = stepWalk(write);
            assert(fault == NoFault || read == NULL);
            state = nextState;
            nextState = Ready;
            if (write)
                walker->port.sendAtomic(write);
        } while (read);
        state = Ready;
        nextState = Waiting;
    }
    return fault;
}

void
Walker::WalkerState::setupWalk(Addr vaddr)
{
    VAddr addr = vaddr;
    CR3 cr3 = tc->readMiscRegNoEffect(misc_reg::Cr3);
    // Check if we're in long mode or not
    Efer efer = tc->readMiscRegNoEffect(misc_reg::Efer);
    dataSize = 8;
    Addr topAddr;
    PageTableEntry pte;
    uint_t hit_num = psc->access(vaddr,pte);
	//all miss
	
	
    if (efer.lma) {
        // Do long mode.
        
		if(hit_num == 4)// L4 PSC hit
		{
			state = LongPDP;
			topAddr = mbits(pte, 51, 12) + vaddr.longl3 * dataSize;
			
		}
		else if(hit_num == 3)//L3 hit
		{
			state = LongPD;
			topAddr = mbits(pte, 51, 12) + vaddr.longl1 * dataSize;
		}
		else if(hit_num == 2)//L2 hit
		{		
			state = LongPTE;
			topAddr = mbits(pte, 51, 12) + vaddr.longl2 * dataSize;	
		}
		else
		{
			state = LongPML4;
			topAddr = (pte.longPdtb << 12) + addr.longl4 * dataSize;
		}
		
        enableNX = efer.nxe;
    } else {
        // We're in some flavor of legacy mode.
        CR4 cr4 = tc->readMiscRegNoEffect(misc_reg::Cr4);
        if (cr4.pae) {
            // Do legacy PAE.
            state = PAEPDP;
            topAddr = (cr3.paePdtb << 5) + addr.pael3 * dataSize;
            enableNX = efer.nxe;
        } else {
            dataSize = 4;
            topAddr = (cr3.pdtb << 12) + addr.norml2 * dataSize;
            if (cr4.pse) {
                // Do legacy PSE.
                state = PSEPD;
            } else {
                // Do legacy non PSE.
                state = PD;
            }
            enableNX = false;
        }
    }

    nextState = Ready;
    entry.vaddr = vaddr;

    Request::Flags flags = Request::PHYSICAL;
    if (cr3.pcd)
        flags.set(Request::UNCACHEABLE);

    RequestPtr request = std::make_shared<Request>(
        topAddr, dataSize, flags, walker->requestorId);

    read = new Packet(request, MemCmd::ReadReq);
    read->allocate();
}
/*****************liu******************/

/****************liu******************/
#include "arch/arm/pagetable.hh"
uint8_t 
access(PageTableEntry &pte,Addr vaddr)
{
    uint8_t hit_table[4];
    schedule(new EventFunctionWrapper([this, pkt]{ access_psc(pte,vaddr,hit_table); },
                                      name() + ".accessEvent", true),
             clockEdge(latency));
    if(access_psc(pte,hit_table))
    {
        for(uint8_t i = 0;i < 4;i++)
        if(hit_table[i] == 1 )
        {
            return i+1;
        }
    }
    return -1;

}
bool
access_psc(PageTableEntry &pte,vaddr,uint8_t *hit_table)
{
    extract_tag(vaddr);
}
/****************liu******************/