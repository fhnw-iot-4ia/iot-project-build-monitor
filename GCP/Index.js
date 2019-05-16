/**
 * Responds to any HTTP request.
 *
 * @param {!express:Request} req HTTP request context.
 * @param {!express:Response} res HTTP response context.
 */
exports.buildMonitorMqttNotification = (req, res) => {
    const mqtt = require('mqtt');
    console.log('mqtt: ', mqtt);
    const broker = 'mqtt://broker.hivemq.com';
    const mqttClient = mqtt.connect(broker);
    console.log('mqttClient: ', mqttClient);
    let status = 'none';
    while (mqttClient === undefined) {
        // wait until client is available
    }
    if (req.body === 'failed') {
         status = '01';
      console.log('setting status to failed');
    } else if (req.body === 'success') {
      status = '00';
      console.log('setting status to success');
    }
    mqttClient.publish('build-monitor/build-status', status);
    res.status(200).send('updated build status');
  };
  