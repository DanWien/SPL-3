package bgu.spl.net.srv;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    ConcurrentHashMap<Integer,ConnectionHandler<T>> getCId_Map();

    ConcurrentHashMap<ConnectionHandler<T>,Integer> getClients_Map();

    ConcurrentHashMap<String,String> getUsers_Passwords_Map();

    ConcurrentHashMap<String,Set<ConnectionHandler<T>>> getTopic_Clients_Map();

    ConcurrentHashMap<Integer,HashMap<String,String>> getCId_TopicTosubId_Map();

    ConcurrentHashMap<Integer,HashMap<String,String>> getCId_subIdToTopic_Map();

    ConcurrentHashMap<Integer,String> getCId_Users_Map();

    ConcurrentHashMap<String,Boolean> getActiveUsers();

    Integer getMessageId();

    
}
