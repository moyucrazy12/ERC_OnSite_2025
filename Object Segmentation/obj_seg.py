import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import numpy as np
import cv2

from ultralytics import YOLO  # YOLOv8 with segmentation support

class ObjectSegmentationNode(Node):
    def __init__(self):
        super().__init__('object_segmentation_node')

        # Subscriptions
        self.rgb_sub = self.create_subscription(Image, 
                                                '/camera/camera/color/image_raw', 
                                                self.rgb_callback, 10)
        self.depth_sub = self.create_subscription(Image, 
                                                  '/camera/camera/depth/image_rect_raw', 
                                                  self.depth_callback, 10)

        self.bridge = CvBridge()
        self.model = YOLO('yolov8n-seg.pt')  # You can use yolov8m-seg.pt or your custom model

        self.rgb_image = None
        self.depth_image = None

        self.get_logger().info("Object Segmentation Node Started")

    def rgb_callback(self, msg):
        self.rgb_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
        self.try_process()

    def depth_callback(self, msg):
        depth = self.bridge.imgmsg_to_cv2(msg, desired_encoding='passthrough')
        if depth.dtype != np.float32:
            depth = depth.astype(np.float32) / 1000.0  # Convert from mm to meters if needed
        self.depth_image = depth
        self.try_process()

    def try_process(self):
        if self.rgb_image is None or self.depth_image is None:
            return

        rgb = self.rgb_image.copy()
        depth = self.depth_image.copy()

        # Run YOLO segmentation
        results = self.model(rgb)

        for result in results:
            if result.masks is None:
                continue

            boxes = result.boxes
            masks = result.masks.data.cpu().numpy()  # N x H x W
            class_ids = boxes.cls.cpu().numpy()
            names = result.names

            for i, mask in enumerate(masks):
                label = names[int(class_ids[i])]
                masked_depth = depth[mask.astype(bool)]
                valid_depth = masked_depth[~np.isnan(masked_depth)]
                valid_depth = valid_depth[valid_depth > 0.1]  # Remove close zero/noise values

                if valid_depth.size == 0:
                    distance = float('nan')
                else:
                    distance = np.median(valid_depth)

                self.get_logger().info(f"{label}: {distance:.2f} m")

                # Optionally draw
                rgb[mask.astype(bool)] = (0, 255, 0)
                box = boxes[i].xyxy[0].cpu().numpy().astype(int)
                cv2.rectangle(rgb, (box[0], box[1]), (box[2], box[3]), (255, 0, 0), 2)
                cv2.putText(rgb, f'{label}: {distance:.2f}m', 
                            (box[0], box[1] - 10), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

        # Show output (for debug)
        cv2.imshow('Segmented Image', rgb)
        cv2.waitKey(1)

def main(args=None):
    rclpy.init(args=args)
    node = ObjectSegmentationNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
    cv2.destroyAllWindows()

if __name__ == '__main__':
    main()
