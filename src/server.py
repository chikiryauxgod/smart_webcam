import grpc
from concurrent import futures
import numpy as np
import cv2
from ultralytics import YOLO
import time
import sys

from generated.proto import video_processor_pb2
from generated.proto import video_processor_pb2_grpc

class YOLOModel:
    def __init__(self, model_path="yolov8n.pt"):
        self.model = YOLO(model_path)
        self.unknown_start_time = None
        self.unknown_duration_threshold = 5  # 5 seconds to approve a stranger

    def process(self, image):
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        results = self.model(image_rgb)
        detections = []
        has_unknown_person = False

        for result in results:
            for box in result.boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                conf = float(box.conf)
                cls = int(box.cls)
                label = result.names[cls]
                if label == "person":
                    has_unknown_person = True
                    detections.append(f"Detected a stranger: [{x1}, {y1}, {x2}, {y2}] ({conf:.2f})")

        return detections, has_unknown_person

    def track_unknown(self, has_unknown_person):
        current_time = time.time()
        if has_unknown_person:
            if self.unknown_start_time is None:
                self.unknown_start_time = current_time
            elif current_time - self.unknown_start_time >= self.unknown_duration_threshold:
                return True
        else:
            self.unknown_start_time = None
        return False

class VideoProcessorServicer(video_processor_pb2_grpc.VideoStreamServicer):
    def __init__(self):
        self.ai_model = YOLOModel()

    def StreamVideo(self, request_iterator, context):
        for frame in request_iterator:
            image_array = np.frombuffer(frame.image_data, dtype=np.uint8)
            image = image_array.reshape(frame.height, frame.width, 3)
            if image is None:
                continue

            detections, has_unknown_person = self.ai_model.process(image)
            if self.ai_model.track_unknown(has_unknown_person):
                result = video_processor_pb2.Result(data="Detected a stranger: " + ", ".join(detections))
                yield result
                time.sleep(5)  

def serve(port):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    video_processor_pb2_grpc.add_VideoStreamServicer_to_server(VideoProcessorServicer(), server)
    server.add_insecure_port(f'[::]:{port}')
    print(f"Python server started on port {port}")
    server.start()
    server.wait_for_termination()  

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python server.py <port>")
        sys.exit(1)
    port = sys.argv[1]
    serve(port)