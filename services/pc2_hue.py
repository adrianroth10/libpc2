#!/usr/bin/env python
import pika

import phue
#import bo
#source_dict = {y: x for x, y in bo.source_list}
#print source_dict

username = 'OyjBEVhpMlQhVd3sZurhv0iPDLQSqugAZqPxWk52'
huebridge = phue.Bridge('philips-hue', username=username)

hue_groups = huebridge.get_group()
print hue_groups

light_source = False
connection = pika.BlockingConnection(pika.ConnectionParameters(host='localhost'))
channel = connection.channel()

channel.exchange_declare(exchange='PC2', exchange_type='topic', durable=True)

result = channel.queue_declare(exclusive=True)
queue_name = result.method.queue
channel.queue_bind(exchange='PC2', queue=queue_name, routing_key='LIGHT.Beo4')

source_queue = channel.queue_declare(exclusive=True)
source_queue_name = source_queue.method.queue
channel.queue_bind(exchange='PC2', queue=source_queue_name, routing_key='new_source')

print(' [*] Waiting for messages. To exit press CTRL+C')

def new_source(ch, method, properties, body):
    print 'callback', body
    if body == '29':
        print('dvd selected')
        huebridge.run_scene("Living room", "TV")


def callback(ch, method, properties, body):
    global light_source, huebridge
    print(" [x] %r" % body)
    print(int(body, base=16))
    if body == "9B":
        light_source = True
    if body == "58":
        light_source = False
    if light_source == True:
        if body == "00":
            huebridge.set_group("Living room", {'on': False})
        if body == "01":
            huebridge.run_scene("Living room", "Hvitt")
        if body == "02":
            huebridge.run_scene("Living room", "Dimmed")
        if body == "03":
            huebridge.run_scene("Living room", "TV")
        pass


channel.basic_consume(callback, queue=queue_name, no_ack=True)
channel.basic_consume(new_source, queue=source_queue_name, no_ack=True)



channel.start_consuming()
