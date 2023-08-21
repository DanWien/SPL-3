package bgu.spl.net.srv;
import java.util.concurrent.ConcurrentHashMap;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;
import java.util.Map.Entry;
import java.util.concurrent.atomic.AtomicInteger;

public class ConnectionsImp<T> implements Connections {

    private AtomicInteger messageId;
    private ConcurrentHashMap<Integer,ConnectionHandler<T>> cId_Client;    // Mapping between connectionID to its ConnectionHandler.
    private ConcurrentHashMap<ConnectionHandler<T>,Integer> client_cId;    // Mapping between ConnectionHandler to its clientId
    private ConcurrentHashMap<String,String> users_Passwords;              // Mapping between usernames and passwords.
    private ConcurrentHashMap<String,Set<ConnectionHandler<T>>> topic_clients;  // Mapping between a topic to the clients that are listening to this topic.
    private ConcurrentHashMap<Integer,HashMap<String,String>> cId_subIdToTopic;  // Mapping between connectionIds to their subIds corresponding to which topic the subID belongs to.
    private ConcurrentHashMap<Integer,HashMap<String,String>> cId_TopicTosubId;  // Mapping between connectionsIds to their topics corresponding to the subID belongs to.
    private ConcurrentHashMap<Integer,String> cId_user;                        // Mapping between a client to the active user.
    private ConcurrentHashMap<String,Boolean> activeUsers;
    // private ArrayList<String> activeUsers;

    public ConnectionsImp(){
        messageId=new AtomicInteger(0);
        cId_Client= new ConcurrentHashMap<>();
        client_cId = new ConcurrentHashMap<>();
        users_Passwords= new ConcurrentHashMap<>();
        topic_clients= new ConcurrentHashMap<>();
        cId_subIdToTopic=new ConcurrentHashMap<>();
        cId_TopicTosubId= new ConcurrentHashMap<>();
        cId_user=new ConcurrentHashMap<>();
        activeUsers = new ConcurrentHashMap<>();
        // activeUsers=new ArrayList<>();
    }

    @Override
    public boolean send(int connectionId, Object msg) {
        ConnectionHandler<T> client= cId_Client.get(connectionId);
        if(client==null)
            return false;
        client.send((T)msg);
        return true;
    }

    @Override
    public void send(String channel, Object msg) {
        Set<ConnectionHandler<T>> subbedClients=topic_clients.get(channel);
        for(ConnectionHandler<T> c: subbedClients)
            c.send((T)msg);
    }

    @Override
    public void disconnect(int connectionId) {
        ConnectionHandler<T> client=cId_Client.get(connectionId);    // Getting the connectionHandler
        cId_Client.remove(connectionId);            // Removing connectionHandler of this connectionID
        client_cId.remove(client);            // Removing connectionHandler of this connectionID
        if(cId_user.containsKey(connectionId)){
            for(Entry<String, Set<ConnectionHandler<T>>> currEntry : topic_clients.entrySet())    // Iterating through all of the topics and removing the client from any topic he is listening to
                currEntry.getValue().remove(client);
            String username = cId_user.remove(connectionId);              // Removing the client from the active user list
            activeUsers.replace(username, true, false);
            cId_TopicTosubId.remove(connectionId);     // Removing the hashMap that matches this client ID
        }     
    }

    public ConcurrentHashMap<Integer,ConnectionHandler<T>> getCId_Map(){
        return cId_Client;
    }
    public ConcurrentHashMap<ConnectionHandler<T>,Integer> getClients_Map(){
        return client_cId;
    }
    public ConcurrentHashMap<String,String> getUsers_Passwords_Map(){
        return users_Passwords;
    }
    public ConcurrentHashMap<String,Set<ConnectionHandler<T>>> getTopic_Clients_Map(){
        return topic_clients;
    }
    public ConcurrentHashMap<Integer,HashMap<String,String>> getCId_TopicTosubId_Map(){
        return cId_TopicTosubId;
    }
    public ConcurrentHashMap<Integer,String> getCId_Users_Map(){
        return cId_user;
    }
    public ConcurrentHashMap<Integer,HashMap<String,String>> getCId_subIdToTopic_Map(){
        return cId_subIdToTopic;
    }
    public ConcurrentHashMap<String,Boolean> getActiveUsers(){
        return activeUsers;
    }
    
    public Integer getMessageId() {
        Integer output = messageId.getAndAdd(1);
        return output;
    }

}
