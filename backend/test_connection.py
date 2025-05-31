from aiokafka import AIOKafkaConsumer
import asyncio
from aiokafka import AIOKafkaConsumer
import asyncio
import logging
from app.redis_broker.config import KAFKA_BOOTSTRAP_SERVERS

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("kafka_consumer")
topic = 'chat_13'


async def consume_messages():
    try:
        logger.info(f"Connecting to Kafka at {KAFKA_BOOTSTRAP_SERVERS}...")

        consumer = AIOKafkaConsumer(
	  topic,
	  bootstrap_servers=KAFKA_BOOTSTRAP_SERVERS,
	  group_id="chat-consumer-group",  # Обязательно укажите group_id!
	  auto_offset_reset='earliest',  # Начинать чтение с начала
	  enable_auto_commit=True,  # Автокоммит оффсетов
	  session_timeout_ms=10000,  # Таймаут сессии
	  max_poll_interval_ms=300000  # Макс. интервал опроса
        )

        await consumer.start()
        logger.info(f"Subscribed to topic: {topic}")

        try:
           async for msg in consumer:
              logger.info(
		f"Received: topic={msg.topic} partition={msg.partition} "
		f"offset={msg.offset} key={msg.key} value={msg.value.decode()}"
	      )
        except Exception as e:
           logger.error(f"Error in consumption: {str(e)}")
        finally:
            logger.warning("Stopping consumer...")
            await consumer.stop()

    except Exception as e:
        logger.exception(f"Fatal error: {str(e)}")


if __name__ == '__main__':
    asyncio.run(consume_messages())