'''
Sample Usage:-
python pose_estimation.py --K_Matrix calibration_matrix.npy --D_Coeff distortion_coefficients.npy --type DICT_5X5_100
'''


import numpy as np
import cv2
import sys
from utils import ARUCO_DICT
import argparse
import time


def pose_estimation(frame, aruco_dict_type, matrix_coefficients, distortion_coefficients):
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    aruco_dict = cv2.aruco.getPredefinedDictionary(aruco_dict_type)
    parameters = cv2.aruco.DetectorParameters()
    detector = cv2.aruco.ArucoDetector(aruco_dict, parameters)

    corners, ids, _ = detector.detectMarkers(gray)

    if len(corners) > 0:
        for i in range(len(ids)):
            # Define the 3D coordinates of the marker corners in the world
            marker_length = 0.02  # meters
            obj_points = np.array([
                [-marker_length / 2,  marker_length / 2, 0],
                [ marker_length / 2,  marker_length / 2, 0],
                [ marker_length / 2, -marker_length / 2, 0],
                [-marker_length / 2, -marker_length / 2, 0]
            ], dtype=np.float32)

            image_points = corners[i][0].astype(np.float32)
            success, rvec, tvec = cv2.solvePnP(obj_points, image_points, matrix_coefficients, distortion_coefficients)

            # Draw the marker and axis
            cv2.aruco.drawDetectedMarkers(frame, corners)
            cv2.drawFrameAxes(frame, matrix_coefficients, distortion_coefficients, rvec, tvec, 0.01)

    return frame

if __name__ == '__main__':

    ap = argparse.ArgumentParser()
    ap.add_argument("-k", "--K_Matrix", required=True, help="Path to calibration matrix (numpy file)")
    ap.add_argument("-d", "--D_Coeff", required=True, help="Path to distortion coefficients (numpy file)")
    ap.add_argument("-t", "--type", type=str, default="DICT_ARUCO_ORIGINAL", help="Type of ArUCo tag to detect")
    args = vars(ap.parse_args())

    
    if ARUCO_DICT.get(args["type"], None) is None:
        print(f"ArUCo tag type '{args['type']}' is not supported")
        sys.exit(0)

    aruco_dict_type = ARUCO_DICT[args["type"]]
    calibration_matrix_path = args["K_Matrix"]
    distortion_coefficients_path = args["D_Coeff"]
    
    k = np.load(calibration_matrix_path)
    d = np.load(distortion_coefficients_path)

    video = cv2.VideoCapture(0)
    time.sleep(2.0)

    while True:
        ret, frame = video.read()

        if not ret:
            break
        
        output = pose_estimation(frame, aruco_dict_type, k, d)

        cv2.imshow('Estimated Pose', output)

        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break

    video.release()
    cv2.destroyAllWindows()