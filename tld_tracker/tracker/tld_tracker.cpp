#include <tracker/tld_tracker.h>

namespace TLD {

TldTracker::TldTracker(Settings settings)
    : _settings(settings), _integrator(_model) {
    _processing_en = false;
}

Candidate TldTracker::SetFrame(const cv::Mat& input_frame) {
    _src_frame = input_frame.clone();
    cv::GaussianBlur(_src_frame, _lf_frame, cv::Size(7,7), 7);

    _detector.SetFrame(std::make_shared<cv::Mat>(_lf_frame));
    _model.SetFrame(std::make_shared<cv::Mat>(_src_frame));
    _tracker.SetFrame(_src_frame);

    if (_processing_en) {
        _detector_proposals = _detector.Detect();
        _tracker_proposal = _tracker.Track();
        std::cout << "Object model prediction: " << _model.Predict(_tracker_proposal) << std::endl;
        std::tie(_prediction, _training_en) = _integrator.Integrate(_detector_proposals, _tracker_proposal);
        if (_training_en) {
            _detector.Train(_prediction);
            _model.Train(_prediction);
        }
        _tracker.SetTarget(_prediction.strobe);
    }
    return _prediction;
}

void TldTracker::StartTracking(const cv::Rect target) {
    _detector.SetTarget(target);
    _tracker.SetTarget(target);
    _model.SetTarget(target);
    _processing_en = true;
}

void TldTracker::StopTracking() {
    _processing_en = false;
}



void TldTracker::UpdateSettings() {

}

bool TldTracker::IsProcessing() {
    return _processing_en;
}

std::tuple<std::vector<Candidate>, Candidate> TldTracker::GetProposals() {
    return make_tuple(_detector_proposals, _tracker_proposal);
}

}




