package bgu.spl.net.impl.stomp;
import bgu.spl.net.srv.Reactor;
import bgu.spl.net.srv.Server;

public class StompServer {
    public static void main(String[] args) {

        if(args[1].equals("reactor"))
            Server.reactor(Runtime.getRuntime().availableProcessors() , Integer.decode(args[0]), () -> new StompMessagingProtocolImp() , () -> new StompEncoderDecoder()).serve();
        else if(args[1].equals("tpc"))
            Server.threadPerClient(Integer.parseInt(args[0]), () -> new StompMessagingProtocolImp(), () -> new StompEncoderDecoder()).serve();
    }
}
