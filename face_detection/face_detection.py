from ultralytics import YOLO
import cv2
import torch

device = 'cuda' if torch.cuda.is_available() else 'cpu'
print("Using device: ", device)


# Define the source: 0 for webcam or the path to your video file
VIDEO_SOURCE = 0
VIDEO_PATH = "videos/sample.mp4"

cap = cv2.VideoCapture(VIDEO_SOURCE)

# Load the YOLO model
chosen_model = YOLO('weights/best_10k_img.pt')  # Adjust model version as needed .pt .onnx
chosen_model.to(device)


def predict(chosen_model, img, conf=0.4):
    results = chosen_model.predict(img, conf=conf, device=device, stream=True, verbose=False)
    return results


def predict_and_detect(chosen_model, img, conf=0.4):
    global face_id, person_id

    # Reset ID mỗi frame
    face_id = 0
    person_id = 0

    results = predict(chosen_model, img, conf)

    for result in results:
        for box in result.boxes:
            cls_name = result.names[int(box.cls[0])]
            confidence = float(box.conf[0])      # lấy confidence
            x1, y1, x2, y2 = map(int, box.xyxy[0])

            if cls_name == "face":
                face_id += 1
                label = f"face_{face_id} ({confidence:.2f})"
                color = (0, 255, 0)

            # elif cls_name == "person":
            #     person_id += 1
            #     label = f"person_{person_id} ({confidence:.2f})"
            #     color = (0, 0, 255)

            else:
                continue

            # Draw bounding box
            cv2.rectangle(img, (x1, y1), (x2, y2), color, 1)

            # Draw label (text)
            cv2.putText(img, label, (x1, y1 - 5),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.5, color, 1)

    return img, results


while True:
    success, img = cap.read()

    if not success:
        break

    img = cv2.resize(img, (800, 600))
    result_img, _ = predict_and_detect(chosen_model, img, conf=0.4)

    # Display the frame with bounding boxes
    cv2.imshow("Image", result_img)

    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

# Release video writer and capture objects
cap.release()
cv2.destroyAllWindows()

