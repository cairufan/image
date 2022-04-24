
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>

#define DEFAULT_WIDTH  (1280)
#define DEFAULT_HEIGHT (720)
#define DEFAULT_FORMAT V4L2_PIX_FMT_NV12 //V4L2_PIX_FMT_YUYV
#define DEFAULT_FIELD  V4L2_FIELD_ANY

#define DEFAULT_DEV "/dev/video0"
#define DEFAULT_WRITE_FILENAME "cap_video0.yuv"
#define DEFAULT_WRITE_FRAME_NUM (1)

#define FMT_NUM_PLANES (1)

// static int verbose = 0;
// #define pr_debug(fmt, arg...) \
//     if (verbose) LOG_ERR(fmt, ##arg)

#define CLEAR(x) memset(&(x), 0, sizeof(x)) //相当于CLEAR(x) =memset(&(x), 0, sizeof(x))



#define LOG_ERR(fmt, args ...) \ printf(fmt, ##args);
#define LOG_WRN(fmt, args ...) \ printf(fmt, ##args);
#define LOG_IFO(fmt, args ...) \ printf(fmt, ##args);
#define LOG_DBG(fmt, args ...) \ printf(fmt, ##args);
#define LOG_ERR_TAG(fmt, args ...) \ printf(fmt, ##args);
#define LOG_WRN_TAG(fmt, args ...) \ printf(fmt, ##args);
#define LOG_IFO_TAG(fmt, args ...) \ printf(fmt, ##args);
#define LOG_DBG_TAG(fmt, args ...) \ printf(fmt, ##args);
#define RETURN_errno(fmt, args ...) \ printf(fmt, ##args);


static int DoIOCtl(int fh, int request, void* arg)
{
    int ret;

    do {
        ret = ioctl(fh, request, arg);
    } while (-1 == ret && EINTR == errno);

    return ret;
}



class V4L2Capture
{
public:
    struct UserBuffer {
        void*   start;
        size_t  length;
    };

    enum IO_METHOD {
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
    };

    enum { _ReqBufNum = 12, };

public:
    V4L2Capture(const std::string& devName = DEFAULT_DEV, const IO_METHOD io = IO_METHOD_MMAP);
    ~V4L2Capture();

    int Init();


    // int GetFrame();
    // int ReleaseFrame();
    int mainloop();

    void SetWidthHeight(const int width, const int height)
    {
        width_ = width;
        height_ = height;
    }


private:
    int OpenDevice();

    int InitDevice();

    int StartCapture();

    int Init_mmap();

    void ProcessImage(const void* p, int size);
    int ReadFrame();

public:
    std::string            idTag_;
    bool                   isInit_ = false;
    bool                   isStreamOn_ = false;
    std::string            devName_;
    enum IO_METHOD         io_ = IO_METHOD_MMAP;
    int                    v4l2Fd_ = -1;
    struct UserBuffer*     buffers_;
    unsigned int           n_buffers_;
    int                    out_buf_;
    int                    force_format_ = true;
    int                    width_ = DEFAULT_WIDTH;
    int                    height_ = DEFAULT_HEIGHT;
    unsigned int           readFrameCnt_ = (-1);
    int                    wrFrameCnt_ = 1;
    std::string            writeFileName_ = "VideoCapture.yuv";
    bool                   isWriteToFile_ = false;
    FILE*                  fpWriteOut_ = NULL; //保存视频
    struct v4l2_capability v4l2Capability_;
};

V4L2Capture::V4L2Capture(const std::string& devName, const IO_METHOD io)
    : idTag_(devName), devName_(devName), io_(io)
{

}

V4L2Capture::~V4L2Capture()
{
    DeInit();
}

int V4L2Capture::Init()
{
    if (!isInit_) {
        int ret = 0;
        ret |= OpenDevice();
        ret |= InitDevice();
        ret |= StartCapture();

        if (0 == ret) {
            isInit_ = true;
            LOG_IFO_TAG("init suc\n");
        } else {
            LOG_ERR_TAG("init failed\n");
        }
    } else {
        LOG_IFO_TAG("device alrady init\n");
    }
    return 0;
}

int V4L2Capture::StartCapture()
{
    unsigned int i;
    enum v4l2_buf_type type;
    int err;


    LOG_DBG_TAG("\tn_buffers: %d\n", n_buffers_);

    switch (io_) {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
            for (i = 0; i < n_buffers_; ++i) {
                struct v4l2_buffer buf;

                LOG_DBG_TAG("\ti: %d\n", i);

                CLEAR(buf);

                if (v4l2Capability_.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
                else
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                struct v4l2_plane planes[FMT_NUM_PLANES];
                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf.type) {
                    buf.m.planes = planes;
                    buf.length = FMT_NUM_PLANES;
                }
                LOG_DBG_TAG("\tbuf.index: %d\n", buf.index);

                err = DoIOCtl(v4l2Fd_, VIDIOC_QBUF, &buf);

                if (-1 == err)
                    RETURN_errno("VIDIOC_QBUF");

                LOG_DBG_TAG("\tbuffer had queued!, ret:%d\n", err);
            }

            LOG_DBG_TAG("Before STREAMON\n");
            if (v4l2Capability_.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            else
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == DoIOCtl(v4l2Fd_, VIDIOC_STREAMON, &type))
                RETURN_errno("VIDIOC_STREAMON");
            LOG_DBG_TAG("After STREAMON\n");
            break;

        case IO_METHOD_USERPTR:

            break;
    }
    return 0;
}
int V4L2Capture::OpenDevice()
{
    struct stat st;

    if (-1 == stat(devName_.c_str(), &st)) {
        LOG_ERR_TAG("Cannot identify '%s': %d, %s\n",
                    devName_.c_str(), errno, strerror(errno));
        return -1;
    }

    if (!S_ISCHR(st.st_mode)) {
        LOG_ERR_TAG("%s is not device\n", devName_.c_str());
        return -2;
    }

    v4l2Fd_ = open(devName_.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == v4l2Fd_) {
        LOG_ERR_TAG("Cannot open '%s': %d, %s\n", devName_.c_str(), errno, strerror(errno));
        return -3;
    }
    LOG_IFO_TAG("open device:%s, suc! v4l2Fd:%d\n", devName_.c_str(), v4l2Fd_);
    return 0;
}

int V4L2Capture::InitDevice()
{
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;


    if (-1 == DoIOCtl(v4l2Fd_, VIDIOC_QUERYCAP, &v4l2Capability_)) {
        if (EINVAL == errno) {
            LOG_ERR_TAG("%s is not V4L2 device\n", devName_.c_str());
            return -1;
        } else {
            RETURN_errno("VIDIOC_QUERYCAP");
        }
    }

    if (!(v4l2Capability_.capabilities & V4L2_CAP_VIDEO_CAPTURE)
        && !(v4l2Capability_.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        LOG_ERR("%s is not video capture device\n", devName_.c_str());
        return -2;
    }

    /* Select video input, video standard and tune here. */

    struct v4l2_format fmt_Get_before_Set;
    CLEAR(fmt_Get_before_Set);
    if (v4l2Capability_.device_caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        fmt_Get_before_Set.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        fmt_Get_before_Set.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == DoIOCtl(v4l2Fd_, VIDIOC_G_FMT, &fmt_Get_before_Set)) {
        LOG_DBG_TAG("xxxxxxxxxx get fail, errno:%d, %s\n", errno, strerror(errno));
    }

    CLEAR(fmt);

    if (v4l2Capability_.device_caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (force_format_) {
        fmt.fmt.pix.width       = width_;//DEFAULT_WIDTH;
        fmt.fmt.pix.height      = height_;//DEFAULT_HEIGHT;
        fmt.fmt.pix.pixelformat = DEFAULT_FORMAT;
        fmt.fmt.pix.field       = DEFAULT_FIELD;

        LOG_DBG("Get fmt, w:%d,H:%d,fmt.fmt.pix.pixelformat: %c,%c,%c,%c\n",
                fmt.fmt.pix.width,
                fmt.fmt.pix.height,
                fmt.fmt.pix.pixelformat & 0xFF,
                (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
                (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
                (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
        LOG_DBG("\n");

        if (-1 == DoIOCtl(v4l2Fd_, VIDIOC_S_FMT, &fmt))
            RETURN_errno("VIDIOC_S_FMT");

        /* Note VIDIOC_S_FMT may change width and height. */

        struct v4l2_format fmtGet;
        CLEAR(fmtGet);
        if (v4l2Capability_.device_caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            fmtGet.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        else
            fmtGet.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == DoIOCtl(v4l2Fd_, VIDIOC_G_FMT, &fmtGet)) {
            LOG_DBG_TAG("xxxxxxxxxx get fail, errno:%d, %s\n", errno, strerror(errno));
        }
        LOG_DBG_TAG("\tGet format, w:%d,H:%d,fmt.fmt.pix.pixelformat: %c,%c,%c,%c\n",
                    fmtGet.fmt.pix.width,
                    fmtGet.fmt.pix.height,
                    fmtGet.fmt.pix.pixelformat & 0xFF,
                    (fmtGet.fmt.pix.pixelformat >> 8) & 0xFF,
                    (fmtGet.fmt.pix.pixelformat >> 16) & 0xFF,
                    (fmtGet.fmt.pix.pixelformat >> 24) & 0xFF);
    } else {
        /* Preserve original settings as set by v4l2-ctl for example */
        if (-1 == DoIOCtl(v4l2Fd_, VIDIOC_G_FMT, &fmt))
            RETURN_errno("VIDIOC_G_FMT");

        if (v4l2Capability_.device_caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        else
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        LOG_DBG("\tfmt.fmt.pix.pixelformat: %c,%c,%c,%c\n",
                fmt.fmt.pix.pixelformat & 0xFF,
                (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
                (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
                (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
    }

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

            Init_mmap();

    return 0;
}


int V4L2Capture::Init_mmap()
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = _ReqBufNum;
    if (v4l2Capability_.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == DoIOCtl(v4l2Fd_, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            LOG_ERR_TAG("%s does not support memory mapping\n", devName_.c_str());
            return -1;
        } else {
            RETURN_errno("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        LOG_ERR_TAG("Insufficient UserBuffer memory on %s\n", devName_.c_str());
        return -2;
    }

    buffers_ = (UserBuffer*)calloc(req.count, sizeof(*buffers_));
    if (!buffers_) {
        LOG_ERR_TAG("Out of memory\n");
        return -3;
    }

    for (n_buffers_ = 0; n_buffers_ < req.count; ++n_buffers_) {
        struct v4l2_buffer buf;

        CLEAR(buf);
        if (v4l2Capability_.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        else
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers_;
        struct v4l2_plane planes[FMT_NUM_PLANES];
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf.type) {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (-1 == DoIOCtl(v4l2Fd_, VIDIOC_QUERYBUF, &buf))
            RETURN_errno("VIDIOC_QUERYBUF");

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf.type) {
            // tmp_buffers[n_buffers_].length = buf.m.planes[0].length;
            buffers_[n_buffers_].length = buf.m.planes[0].length;
            buffers_[n_buffers_].start =
                mmap(NULL /* start anywhere */,
                     buf.m.planes[0].length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     v4l2Fd_, buf.m.planes[0].m.mem_offset);
        } else {
            buffers_[n_buffers_].length = buf.length;
            buffers_[n_buffers_].start =
                mmap(NULL /* start anywhere */,
                     buf.length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     v4l2Fd_, buf.m.offset);
        }

        if (MAP_FAILED == buffers_[n_buffers_].start)
            RETURN_errno("mmap");
    }
    return 0;
}

void V4L2Capture::ProcessImage(const void* p, int size)
{
    //采集视频
    LOG_IFO_TAG("%s, process size:%d,image:%p\n", devName_.c_str(), size, p);

    if (isWriteToFile_ && wrFrameCnt_ > 0) {
        if (!fpWriteOut_) {
            fpWriteOut_ = fopen(writeFileName_.c_str(), "wb");
        }
        if (fpWriteOut_) {
            int writeSize = 0;
            if (1) {
                uint8_t* picture = (uint8_t*)p;
                for (size_t row = 0; row < 1080; row++) {
                    if (row < 720) {
                        fwrite(picture, width_, 1, fpWriteOut_);
                        writeSize += width_;
                    }
                    picture += 1920;
                }
                for (size_t row = 0; row < 1080; row++) {
                    if (row < 720) {
                        fwrite(picture, width_ / 2, 1, fpWriteOut_);
                        writeSize += width_ / 2;
                    }
                    picture += 1920;
                }
            } else {
                writeSize = size;
                fwrite(p, size, 1, fpWriteOut_);
            }
            LOG_DBG_TAG("\twrite size:%d\n", writeSize);
            --wrFrameCnt_;
        }
    }
}

int V4L2Capture::ReadFrame()
{
    struct v4l2_buffer buf;
    // unsigned int i;
    int size = 0, index = 0;

    switch (io_) {
        case IO_METHOD_READ:

            break;

        case IO_METHOD_MMAP:
            CLEAR(buf);
            if (v4l2Capability_.device_caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            else
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            struct v4l2_plane planes[FMT_NUM_PLANES];
            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf.type) {
                buf.m.planes = planes;
                buf.length = FMT_NUM_PLANES;
            }

            if (-1 == DoIOCtl(v4l2Fd_, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                    case EAGAIN:
                        return 0;

                    case EIO:
                    /* Could ignore EIO, see spec. */

                    /* fall through */

                    default:
                        RETURN_errno("VIDIOC_DQBUF");
                }
            }

            assert(buf.index < n_buffers_);

            size = buf.m.planes[0].length & 0x07ffffff;
            index = (buf.m.planes[0].length & 0xf8000000) >> 27;

            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf.type)
                buf.bytesused = buf.m.planes[0].bytesused;

            ProcessImage(buffers_[buf.index].start, buf.bytesused);

            if (-1 == DoIOCtl(v4l2Fd_, VIDIOC_QBUF, &buf))
                RETURN_errno("VIDIOC_QBUF");
            break;

        case IO_METHOD_USERPTR:

            break;
    }

    return 1;
}

int V4L2Capture::mainloop()
{
    unsigned int count;


    count = readFrameCnt_;

    while (count-- > 0) {
        for (;;) {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(v4l2Fd_, &fds);

            /* Timeout. */
            tv.tv_sec = 5;
            tv.tv_usec = 0;

            r = select(v4l2Fd_ + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) {
                if (EINTR == errno)
                    continue;
                RETURN_errno("select");
            }

            if (0 == r) {
                LOG_ERR_TAG("select timeout\n");
                //exit(EXIT_FAILURE);
                continue;
            }

            if (ReadFrame())
                break;
            /* EAGAIN - continue select loop. */
        }
    }

    //DeInit(); //for test

    return 0;
}
int main(int argc, char** argv)
{

    srand(time(NULL));
    int readFrameNum = 0;

    V4L2Capture v4L2Capture3("/dev/video3");
    v4L2Capture3.SetWidthHeight(1920, 1080);


    V4L2Capture v4L2Capture2("/dev/video2");
    v4L2Capture2.SetWidthHeight(1280, 720);

    V4L2Capture v4L2Capture1("/dev/video1");
    v4L2Capture1.SetWidthHeight(1280, 720);

    V4L2Capture v4L2Capture0("/dev/video0");
    v4L2Capture0.SetWidthHeight(1280, 720);

    int runTime = 0;
    while (true) {
        LOG_IFO("----------init capture:%d----------\n", runTime);
        v4L2Capture3.Init();
        v4L2Capture2.Init();
        v4L2Capture1.Init();
        v4L2Capture0.Init();

        LOG_IFO("----------run mainloop:%d----------\n", runTime);
        std::thread t3(std::bind(&V4L2Capture::mainloop, &v4L2Capture3));
        std::thread t2(std::bind(&V4L2Capture::mainloop, &v4L2Capture2));
        std::thread t1(std::bind(&V4L2Capture::mainloop, &v4L2Capture1));
        std::thread t0(std::bind(&V4L2Capture::mainloop, &v4L2Capture0));

        //t3.detach();
        //t2.detach();
        //t1.detach();
        //t0.detach();

        t3.join();
        t2.join();
        t1.join();
        t0.join();

        LOG_IFO("----------join finish:%d----------\n", runTime);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        runTime++;
    }

    LOG_IFO("\nrun over\n");
    sleep(2);

    return 0;
}




