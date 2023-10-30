#include <limits>

#include "gpio.h"

Gpio::Gpio(std::filesystem::path dev) : device_file(dev)
{
    setup();
}

Gpio::~Gpio()
{
    gpiod_chip_close(chip);
}

void Gpio::setup()
{
    chip = gpiod_chip_open(device_file.c_str());

    if (!chip)
        throw std::runtime_error("requested gpiochip could not be opened");

    label = gpiod_chip_label(chip);
}

gpiod_line *Gpio::retrieve_gpiod_line(int line_id)
{
    gpiod_line *line = gpiod_chip_get_line(chip, line_id);

    if (std::string(gpiod_line_consumer(line)) != label)
        throw std::invalid_argument("requested line is not owned by this program");

    return line;
}

void Gpio::set_direction(int line_id, Direction direction)
{
    gpiod_line *line = gpiod_chip_get_line(chip, line_id);

    if (!line)
        throw std::runtime_error("Line " + std::to_string(line_id) + " could not be found");

    if (gpiod_line_is_used(line))
        throw std::runtime_error("Line " + std::to_string(line_id) + " is already in use");

    if (direction == Direction::INPUT)
        gpiod_line_request_input(line, label.c_str());
    else
        gpiod_line_request_output(line, label.c_str(), 0);

    if (!gpiod_line_is_requested(line))
        throw std::runtime_error("Line " + std::to_string(line_id) + " could not be requested");
}

void Gpio::set_output(int line_id, Values value)
{
    gpiod_line *line = retrieve_gpiod_line(line_id);

    if (gpiod_line_direction(line) != GPIOD_LINE_DIRECTION_OUTPUT)
        throw std::runtime_error("requested line is not configured as output");

    int ret = gpiod_line_set_value(line, value);

    if (!ret)
        std::runtime_error("requested line is not changed");
}

Values Gpio::get_input(int line_id)
{
    gpiod_line *line = retrieve_gpiod_line(line_id);

    if (gpiod_line_direction(line) != GPIOD_LINE_DIRECTION_INPUT)
        throw std::runtime_error("requested line is not configured as input");

    return gpiod_line_get_value(line) ? Values::HIGH : Values::LOW;
}

void Gpio::set_detection(int line_id, Detection detection)
{
    gpiod_line *line = gpiod_chip_get_line(chip, line_id);

    // if (gpiod_line_direction(line) != GPIOD_LINE_DIRECTION_INPUT)
    //     throw std::runtime_error("requested line is not configured as input");

    int ret;

    switch (detection)
    {
    case Detection::FALLING:
        ret = gpiod_line_request_falling_edge_events(line, label.c_str());
        break;
    case Detection::RISING:
        ret = gpiod_line_request_rising_edge_events(line, label.c_str());
        break;
    case Detection::BOTH:
        ret = gpiod_line_request_both_edges_events(line, label.c_str());
        break;
    default:
        break;
    }

    if (ret == -1)
        throw std::runtime_error("Could not request edge detection for line " + std::to_string(line_id));
}

// void Gpio::get_event_info(int line_id);

bool Gpio::wait_for_event(int line_id, std::chrono::nanoseconds timeout)
{
    gpiod_line *line = retrieve_gpiod_line(line_id);

    if (gpiod_line_direction(line) != GPIOD_LINE_DIRECTION_INPUT)
        throw std::runtime_error("requested line is not configured as input");

    std::chrono::seconds secs = std::chrono::duration_cast<std::chrono::seconds>(timeout);
    std::chrono::nanoseconds nano = timeout - std::chrono::duration_cast<std::chrono::nanoseconds>(secs);

    // Check if the duration is out of the range of valid values for time_t
    if (secs.count() < std::numeric_limits<decltype(secs)::rep>::min() ||
        secs.count() > std::numeric_limits<decltype(secs)::rep>::max())
    {
        throw std::range_error("Duration is out of range for conversion to timespec");
    }

    timespec timeout_spec = {
        .tv_sec = static_cast<time_t>(secs.count()),
        .tv_nsec = static_cast<long>(nano.count())};

    gpiod_line_event event;

    switch (gpiod_line_event_wait(line, &timeout_spec))
    {
    case 0:
        gpiod_line_event_read(line, &event);
        return false;
    case 1:
        gpiod_line_event_read(line, &event);
        return true;
    default:
        throw std::runtime_error("Could not detect for line " + std::to_string(line_id));
    };
}

void Gpio::release_line(int line_id)
{
    gpiod_line *line = retrieve_gpiod_line(line_id);

    gpiod_line_release(line);
}
