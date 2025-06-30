import json
import boto3

dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('GSR_Sensor_Data') 
def lambda_handler(event, context):
    try:
        body = json.loads(event['body'])

        timestamp = body.get('timestamp')
        gsr = body.get('gsr')
        temp_dht = body.get('temp_dht')
        hum_dht = body.get('hum_dht')
        temp_ds18b20 = body.get('temp_ds18b20')

        if timestamp is None or gsr is None:
            return {
                'statusCode': 400,
                'body': json.dumps({'message': 'Missing required fields: timestamp or gsr'})
            }
            item = {
            'timestamp': timestamp,
            'gsr': gsr,
            'temp_dht': temp_dht,
            'hum_dht': hum_dht,
            'temp_ds18b20': temp_ds18b20
        }
        table.put_item(Item=item)

        return {
            'statusCode': 200,
            'body': json.dumps({'message': 'Data successfully stored!'})
        }

    except Exception as e:
        print(f"Error processing request: {e}")
        return {
            'statusCode': 500,
            'body': json.dumps({'message': f'Error storing data: {str(e)}'})
        }
