package netfilter

import (
	"fmt"
	"log"
	"net"
	"testing"
	"time"
)

var stopCh = make(chan struct{})

func serve(t *testing.T, queueNum uint16) {
	nfq, err := NewNFQueue(queueNum, 100, NF_DEFAULT_PACKET_SIZE)
	if err != nil {
		t.Skipf("Skipping the test due to %s", err)
	}
	defer nfq.Close()
	packets := nfq.GetPackets()

	t.Logf("Starting (NFQ %d)..", queueNum)
	for true {
		select {
		case p := <-packets:
			fmt.Printf("packet:%v\n", p)
			fmt.Printf("net index:%v\n", p.Idx)
			t.Logf("Accepting %s", p.Packet)
			p.SetVerdict(NF_ACCEPT)
		case <-stopCh:
			t.Logf("Exiting..")
			return
		}
	}
}

// very dumb test, but enough for testing golang/go#14210
func TestNetfilter(t *testing.T) {
	ns, err := net.Interfaces()
	if err != nil {
		t.Fatal(err)
	}
	for _, n := range ns {
		log.Printf("idx:%v,name:%v", n.Index, n.Name)
	}

	queueNum := 42
	go serve(t, uint16(queueNum))
	wait := 30 * time.Second
	t.Logf("Sleeping for %s", wait)
	time.Sleep(wait)
	close(stopCh)
}
