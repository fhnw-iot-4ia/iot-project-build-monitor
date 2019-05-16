# Prerequisites
In order to get started you will need the following:
1) Have a GCP Account and project
2) Have the cloud function api enabled

# Code
The code consists of two parts: The `package.json` file and the `index.js` file. Paste them into a new cloud function with the name `buildMonitorMqttNotification` and deploy it. Use the url provided to send post requests to.

Target state | Value to pass
------------ | -------------
`BUILD_SUCCEEDED`  | `success`
`BUILD_FAILED` | `failed`

# Usage
As build job engineer you can simply send a POST request to the function url and pass either one of the values to trigger a build result.
