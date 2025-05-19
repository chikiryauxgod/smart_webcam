import os
import sys

ROOT = os.path.dirname(__file__)
sys.path.insert(0, os.path.join(ROOT, "generated", "proto"))

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
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        results = self.model(image_rgb)
        detections = []
        has_unknown = False

        for result in results:
            for box in result.boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                conf = float(box.conf)
                cls = int(box.cls)
                label = result.names[cls]
                if label == "person":
                    has_unknown = True
                    detections.append(f"[{x1},{y1},{x2},{y2}]({conf:.2f})")

        return detections, has_unknown


import time  # в начало

class VideoProcessorServicer(video_processor_pb2_grpc.VideoProcessorServicer):
    def __init__(self):
        self.ai = YOLOModel()
        self.last_alert_time = 0

    def ProcessVideo(self, request_iterator, context):
        for frame in request_iterator:
            if frame.width == 0 or frame.height == 0:
                continue

            try:
                img = np.frombuffer(frame.image_data, dtype=np.uint8) \
                         .reshape(frame.height, frame.width, 3)
            except Exception:
                continue

            detections, has_unknown = self.ai.process(img)

            now = time.time()
            if has_unknown and now - self.last_alert_time > 5:
                msg = "Detected stranger: " + ", ".join(detections)
                print(msg)
                self.last_alert_time = now
                yield video_processor_pb2.Result(data=msg)



def serve(port):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    video_processor_pb2_grpc.add_VideoProcessorServicer_to_server(
        VideoProcessorServicer(), server
    )
    server.add_insecure_port(f"[::]:{port}")
    print(f"Python server started on port {port}")
    server.start()
    server.wait_for_termination()


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python server.py <port>")
        sys.exit(1)
    serve(sys.argv[1])
