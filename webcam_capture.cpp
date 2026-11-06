#include <gst/gst.h>
#include <iostream>

bool runPipeline(const char* description) {
    GError* error = nullptr;

    GstElement* pipeline =
        gst_parse_launch(description, &error);

    if (error != nullptr) {
        std::cerr << "Pipeline error: "
                  << error->message << std::endl;
        g_error_free(error);

        if (pipeline)
            gst_object_unref(pipeline);

        return false;
    }

    if (!pipeline)
        return false;

    GstBus* bus = gst_element_get_bus(pipeline);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    GstMessage* msg = gst_bus_timed_pop_filtered(
        bus,
        GST_CLOCK_TIME_NONE,
        static_cast<GstMessageType>(
            GST_MESSAGE_ERROR | GST_MESSAGE_EOS
        )
    );

    bool success = false;

    if (msg) {
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
            success = true;
        } else {
            GError* err = nullptr;
            gchar* debug = nullptr;

            gst_message_parse_error(msg, &err, &debug);

            std::cerr << "GStreamer error: "
                      << (err ? err->message : "Unknown")
                      << std::endl;

            if (err) g_error_free(err);
            g_free(debug);
        }

        gst_message_unref(msg);
    }

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return success;
}

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);

    std::cout << "Capturing webcam image..."
              << std::endl;

    const char* capture =
        "v4l2src device=/dev/video0 num-buffers=1 "
        "do-timestamp=true ! "
        "image/jpeg,width=640,height=480,"
        "framerate=15/1 ! "
        "jpegdec ! videoconvert ! "
        "video/x-raw,format=RGB ! "
        "pngenc ! "
        "filesink location=captured.png";

    if (!runPipeline(capture)) {
        std::cerr << "Capture failed!" << std::endl;
        return 1;
    }

    std::cout << "Saved captured.png" << std::endl;
    std::cout << "Converting to grayscale JPEG..."
              << std::endl;

    const char* convert =
        "filesrc location=captured.png ! "
        "pngdec ! videoconvert ! "
        "video/x-raw,format=GRAY8 ! "
        "jpegenc ! "
        "filesink location=grayscale.jpg";

    if (!runPipeline(convert)) {
        std::cerr << "Conversion failed!" << std::endl;
        return 1;
    }

    std::cout << "Saved grayscale.jpg" << std::endl;
    std::cout << "Assignment completed successfully!"
              << std::endl;

    return 0;
}
