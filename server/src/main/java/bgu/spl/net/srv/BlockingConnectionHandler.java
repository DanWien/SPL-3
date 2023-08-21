package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.StompMessagingProtocol;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

public class BlockingConnectionHandler<T> implements Runnable, ConnectionHandler<T> {

    private final StompMessagingProtocol<T> protocol;
    private final MessageEncoderDecoder<T> encdec;
    private final Socket sock;
    private BufferedInputStream in;
    private BufferedOutputStream out;
    private volatile boolean connected = true;
    private ConcurrentLinkedQueue<T> messages;
    private boolean isConnected;
    private static AtomicInteger connectionId=new AtomicInteger(0);
    private ConnectionsImp<T> connections;


    public BlockingConnectionHandler(Socket sock, MessageEncoderDecoder<T> reader, StompMessagingProtocol<T> protocol, ConnectionsImp<T> connections) {
        this.sock = sock;
        this.encdec = reader;
        this.protocol = protocol;
        this.connections=connections;
        isConnected=false;
        messages = new ConcurrentLinkedQueue<>();
    }

    @Override
    public void run() {
        try (Socket sock = this.sock) { //just for automatic closing
            if(!isConnected){
                isConnected=true;
                Integer id=connectionId.getAndAdd(1);
                connections.getCId_Map().put(id,this);
                connections.getClients_Map().put(this,id);
                protocol.start(id, connections);
            }
            
            int read;
            in = new BufferedInputStream(sock.getInputStream());
            out = new BufferedOutputStream(sock.getOutputStream());
            while (!protocol.shouldTerminate() && connected && (read = in.read()) >= 0) {
                T nextMessage = encdec.decodeNextByte((byte) read);
                if (nextMessage != null && nextMessage!=""){
                    protocol.process(nextMessage);
            }
                while (!messages.isEmpty() && connected) {
                    out.write(encdec.encode(messages.poll()));
                    out.flush();
                }  
            }

        } catch (IOException ex) {
            ex.printStackTrace();
        }

    }

    @Override
    public void close() throws IOException {
        connected = false;
        sock.close();
    }

    @Override
    public void send(T msg) {
        messages.add(msg);
    }

}
