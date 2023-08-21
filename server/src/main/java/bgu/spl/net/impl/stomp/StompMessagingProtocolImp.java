package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import bgu.spl.net.srv.ConnectionHandler;

public class StompMessagingProtocolImp implements StompMessagingProtocol<String> {
    private Connections<String> connections;
    private int connectionId;
    private boolean shouldTerminate = false;

    @Override
    public void start(int connectionId, Connections<String> connections) {
        this.connectionId = connectionId;
        this.connections = connections;

    }

    @Override
    public void process(String message) {
        String command = message.substring(0, message.indexOf('\n'));
        String body = message.substring(message.indexOf('\n') + 1);
        HashMap<String, String> headers_values = headerToValue(body);
        boolean isConnected = connections.getCId_Users_Map().containsKey(connectionId);
        boolean hasReceipt = headers_values.containsKey("receipt");
        boolean noError = true;
        if (!isConnected) {
            if (command.equals("CONNECT"))
                noError = handleConnect(headers_values,message,hasReceipt);
            else {
                connections.send(connectionId, "ERROR\nmessage:you need to connect first\n\n");
                noError = false;
            }
        } else {
            if (command.equals("CONNECT")) {
                connections.send(connectionId, "ERROR\nmessage:user is already logged in\n\n");
                noError = false;
            } else if (command.equals("SUBSCRIBE"))
                noError = handleSubscribe(headers_values, message, hasReceipt);
            else if (command.equals("UNSUBSCRIBE"))
                noError = handleUnSubscribe(headers_values, message, hasReceipt);
            else if (command.equals("DISCONNECT"))
                noError = handleDisconnect(headers_values, message, hasReceipt);
            else if (command.equals("SEND"))
                noError = handleSend(headers_values, message, hasReceipt);
            else {
                connections.send(connectionId, "ERROR\nmessage:Illegal command has been sent\n\n");
                noError = false;
            }
        }
        if (hasReceipt) {
            connections.send(connectionId, receiptFrame(headers_values.get("receipt"))); 
            if(command.equals("DISCONNECT")) {
                connections.disconnect(connectionId);
                shouldTerminate = true;
            }

        } 
        else if(!noError) {
            connections.disconnect(connectionId);
            shouldTerminate = true;
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    public String connectedFrame() {
        return "CONNECTED" + '\n' + "version:1.2" + '\n' + '\n';
    }

    public String errorFrame(String receiptId, String reason, String originalMessage, String details) { 
        String output;
        if (!receiptId.equals("-1"))
             output = "ERROR" + '\n' + "receipt-id:"+receiptId + '\n' + reason + '\n' + '\n' + "The message:" +'\n' + "-----\n" + originalMessage + "\n-----\n" + details;
        else
            output =  "ERROR" + '\n' + reason + '\n' + '\n' + "The message:" +'\n' + "-----\n" + originalMessage + "\n-----\n" + details;
        return output;
    }

    public String receiptFrame(String id) {
        return "RECEIPT" + '\n' + "receipt-id:" + id + '\n' + '\n';
    }

    public String messageFrame(String id, String body, String subId, String destination) {
        return "MESSAGE\n"+"subscription:"+subId+'\n'+"message-id:"+id+'\n'+ "destination:"+destination+'\n'+'\n'+body;
    }

    public boolean handleConnect(HashMap<String, String> headers_values, String message, boolean hasReceipt) {
        if (!headers_values.containsKey("accept-version")) {
            if (!hasReceipt) {
                String error = errorFrame("-1", "message:malformed frame received", message,
                        "header accept-version missing ");
                connections.send(connectionId, error);
            } else {
                String error = errorFrame(headers_values.get("receipt"), "message:malformed frame received",
                        message, "header accept-version missing");
                connections.send(connectionId, error);
            }
            return false;
        } else if (!headers_values.containsKey("host")) {
            if (!hasReceipt) {
                String error = errorFrame("-1", "message:malformed frame received", message,
                        "header host missing");
                connections.send(connectionId, error);
            } else {
                String error = errorFrame(headers_values.get("receipt"), "message:malformed frame received",
                        message, "header host missing");
                connections.send(connectionId, error);
            }
            return false;
        }
        else if (!headers_values.containsKey("login")) {
            if (!hasReceipt) {
                String error = errorFrame("-1", "message:malformed frame received", message,
                        "header login missing");
                connections.send(connectionId, error);
            } else {
                String error = errorFrame(headers_values.get("receipt"), "message:malformed frame received",
                        message, "header login missing");
                connections.send(connectionId, error);
            }
            return false;
        }
        else if (!headers_values.containsKey("passcode")) {
            if (!hasReceipt) {
                String error = errorFrame("-1", "message:malformed frame received", message,
                        "header passcode missing");
                connections.send(connectionId, error);
            } else {
                String error = errorFrame(headers_values.get("receipt"), "message:malformed frame received",
                        message, "header passcode missing");
                connections.send(connectionId, error);
            }
            return false;
        }
        String username = headers_values.get("login");
        if(connections.getActiveUsers().contains(username) && connections.getActiveUsers().get(username)) {    // Checking if user is already active
            if(hasReceipt) {
                String error = errorFrame(headers_values.get("receipt"), "message:user is already logged in", message, "You can not login to an active user");
                connections.send(connectionId, error);
            }
            else {
                String error = errorFrame("-1", "message:user is already logged in", message, "You can not login to an active user");
                connections.send(connectionId, error);
            }
            return false;
        }
        else if(connections.getUsers_Passwords_Map().containsKey(username)) {   // Checking if user exists in server
            if(!headers_values.get("passcode").equals(connections.getUsers_Passwords_Map().get(username))) {  // Incorrect password
                if(hasReceipt) {
                    String error = errorFrame(headers_values.get("receipt"), "message:wrong password", message, "Password does not match the input username");
                    connections.send(connectionId, error);
                }
                else {
                    String error = errorFrame("-1", "message:wrong password", message, "Password does not match the input username");
                    connections.send(connectionId, error); 
                }
            return false;
            }
            else {    // User exists and gave the right password
                connections.getActiveUsers().replace(username, false, true);
                connections.getCId_subIdToTopic_Map().put(connectionId , new HashMap<>());
                connections.getCId_TopicTosubId_Map().put(connectionId , new HashMap<>());
                connections.getCId_Users_Map().put(connectionId,username);
                connections.send(connectionId,connectedFrame());
                return true;
            }
       
        }
        else { // User doesn't exist - creating a new one.
            connections.getUsers_Passwords_Map().put(username,headers_values.get("passcode"));
            connections.getActiveUsers().put(username, true);
            connections.getCId_subIdToTopic_Map().put(connectionId , new HashMap<>());
            connections.getCId_TopicTosubId_Map().put(connectionId , new HashMap<>());
            connections.getCId_Users_Map().put(connectionId,username);
            connections.send(connectionId,connectedFrame());
        }    
        return true;
    }

    public boolean handleSubscribe(HashMap<String, String> headers_values, String message, boolean hasReceipt) {
        if (!headers_values.containsKey("destination")) {
            if (hasReceipt) {
                String error = errorFrame(headers_values.get("receipt"), "message:malformed frame received", message,
                        "header destination is missing");
                connections.send(connectionId, error);
            } else {
                String error = errorFrame("-1", "message:malformed frame received", message,
                        "header destination is missing");
                connections.send(connectionId, error);
            }
            return false;
        } 
        else if (!headers_values.containsKey("id")) {
            if (hasReceipt) {
                String error = errorFrame(headers_values.get("receipt"), "message:malformed frame received", message,
                        "header id is missing");
                connections.send(connectionId, error);
            } else {
                String error = errorFrame("-1", "message:malformed frame received", message, "header id is missing");
                connections.send(connectionId, error);
            }
            return false;
        } 
        else {
            String topic = headers_values.get("destination");
            ConcurrentHashMap<String, Set<ConnectionHandler<String>>> topics = connections.getTopic_Clients_Map();
            if (!topics.containsKey(topic))  // Topic doesn't exist
                topics.put(topic, new HashSet<>());  // Creating the topic
            topics.get(topic).add(connections.getCId_Map().get(connectionId));    // Adding the client(connectionHandler) to this topic.
            String subId = headers_values.get("id");
            connections.getCId_subIdToTopic_Map().get(connectionId).put(subId,topic); // Inserting the subID to the client's list of subs.
            connections.getCId_TopicTosubId_Map().get(connectionId).put(topic,subId);
        }
        return true;
    }

    public boolean handleUnSubscribe(HashMap<String, String> headers_values, String message, boolean hasReceipt) {
        if (!headers_values.containsKey("id")) {
            if (hasReceipt) {
                String error = errorFrame(headers_values.get("receipt"), "message:malformed frame received", message,
                        "header id is missing");
                connections.send(connectionId, error);
            } else {
                String error = errorFrame("-1", "message:malformed frame received", message, "header id is missing");
                connections.send(connectionId, error);
            }
            return false;
        }
        else {
            String topic = connections.getCId_subIdToTopic_Map().get(connectionId).get(headers_values.get("id"));
            if(topic == null) {
                if (hasReceipt) {
                    String error = errorFrame(headers_values.get("receipt"), "message:Unauthorized action has been made", message,
                            "You can't unsubscribe from a channel you have not subscribed to");
                    connections.send(connectionId, error);
                } else {
                    String error = errorFrame("-1", "message:Unauthorized action has been made", message, "You can't unsubscribe from a channel you have not subscribed to");
                    connections.send(connectionId, error);
                }
                return false; 
            }
            //need to add a case that a client is trying to unsubscribe from a topic that he is not subscribe to
            connections.getCId_subIdToTopic_Map().get(connectionId).remove(headers_values.get("id"));
            connections.getCId_TopicTosubId_Map().get(connectionId).remove(topic);
            connections.getTopic_Clients_Map().get(topic).remove(connections.getCId_Map().get(connectionId));
        }
        return true;
    }

    public boolean handleDisconnect(HashMap<String, String> headers_values, String message, boolean hasReceipt) {
        if(!hasReceipt) {
            connections.send(connectionId, errorFrame("-1" , "message:malformed frame received" , message , "header receipt is missing"));
            return false;
        }
        // connections.disconnect(connectionId);
        return true;
    }

    public boolean handleSend(HashMap<String, String> headers_values, String message, boolean hasReceipt) {
        if (!headers_values.containsKey("destination")) {
            if (hasReceipt) {
                String error = errorFrame(headers_values.get("receipt"), "message:malformed frame received", message,
                        "header destination is missing");
                connections.send(connectionId, error);
            } else {
                String error = errorFrame("-1", "message:malformed frame received", message, "header destination is missing");
                connections.send(connectionId, error);
            }
            return false;
        }
        String topic = headers_values.get("destination");
        ConnectionHandler<String> c = connections.getCId_Map().get(connectionId);
        if(!connections.getTopic_Clients_Map().get(topic).contains(c)){
            if (hasReceipt) {
                String error = errorFrame(headers_values.get("receipt"), "message:No subscription to destination", message,
                        "You need to be subscribed to a topic in order to broadcast a message");
                connections.send(connectionId, error);
            } else {
                String error = errorFrame("-1", "message:No subscription to destination", message, "You need to be subscribed to a topic in order to broadcast a message");
                connections.send(connectionId, error);
            }
            return false;
        }
        Set<ConnectionHandler<String>> clients=connections.getTopic_Clients_Map().get(topic);
        String id=""+connections.getMessageId();
        for(ConnectionHandler<String> cl: clients){
            String subId=connections.getCId_TopicTosubId_Map().get(connectionId).get(topic);
            int cId = connections.getClients_Map().get(cl);
            String messageToSend = messageFrame(id, headers_values.get("body"), subId, topic);
            connections.send(cId,messageToSend); 
        }
        return true;
    }

    public HashMap<String, String> headerToValue(String body) {
        HashMap<String, String> output = new HashMap<>();
        String line = body.substring(0, body.indexOf('\n'));
        while (line.length() > 0) {
            output.put(line.substring(0, line.indexOf(":")), line.substring(line.indexOf(":") + 1));
            body = body.substring(line.length() + 1);
            line = body.substring(0, body.indexOf('\n'));
        }
        body = body.substring(1);
        output.put("body", body);
        return output;
    }
}
