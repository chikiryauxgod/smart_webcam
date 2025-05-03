import grpc
from concurrent import futures
import numpy as np
import cv2
from ultralytics import YOLO
import video_processor_pb2
import video_processor_pb2_grpc

class YOLOModel:
    def __init__(self, model_path="yolov8n.pt"):
        self.model = YOLO(model_path)  

    def process(self, image):
        # BGR (OpenCV) to RGB (YOLO)
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        results = self.model(image_rgb)
        detections = []
        for result in results:
            for box in result.boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                conf = float(box.conf)
                cls = int(box.cls)
                label = result.names[cls]
                detections.append(f"{label}: [{x1}, {y1}, {x2}, {y2}] ({conf:.2f})")
        return detections

class VideoProcessorServicer(video_processor_pb2_grpc.VideoProcessorServicer):
    def __init__(self):
        self.ai_model = YOLOModel()

    def ProcessVideo(self, request_iterator, context):
        for frame in request_iterator:
            # Decode from JPEG
            image_array = np.frombuffer(frame.image_data, dtype=np.uint8)
            image = cv2.imdecode(image_array, cv2.IMREAD_COLOR)
            if image is None:
                continue

            detections = self.ai_model.process(image)

            result = video_processor_pb2.Result(
                detections=[video_processor_pb2.Detection(data=d) for d in detections]
            )
            yield result

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    video_processor_pb2_grpc.add_VideoProcessorServicer_to_server(
        VideoProcessorServicer(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    server.wait_for_termination()

if __name__ == '__main__':
    serve()