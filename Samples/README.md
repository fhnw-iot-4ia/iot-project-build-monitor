# Prerequisites
In order to get started you will need the following:
1) Have a deployed Continuous Integration server

# Jenkins
If you work with Jenkins you can use the [Jenkinsfile](./Jenkinsfile). All you have to do is the following:
1) Install the mqtt [notification plugin](https://wiki.jenkins.io/display/JENKINS/MQTT+Notification+Plugin)
2) Put your Jenkinsfile under source code control
3) Create a new Item of type Pipeline, choose Jenkinsfile from scm and point to the Jenkinsfile in your scm.

# Other CI Servers
If you are working with a different CI server, you can trigger build results to mqtt using the cloud function: call the deployed cloud function using a http post request with the payload `success` or `failed`.