#include <tracker/object_model.h>

namespace TLD {

    ObjectModel::ObjectModel() :
        _patch_size(15,15),
        _scales({0.75, 0.875, 1.0, 1.125, 1.25}),
        _overlap(0.1),
        _sample_max_depth(100) {
    }

    void ObjectModel::SetFrame(std::shared_ptr<cv::Mat> frame) {
        _frame = frame;
    }

    void ObjectModel::SetTarget(cv::Rect target) {
        _target = target;

        const cv::Mat& frame = *_frame;

        TranformPars aug_pars;
        aug_pars.angles = {-15, 0, 15};
        aug_pars.scales = _scales;
        aug_pars.translation_x = {static_cast<int>(-0.5*_overlap * _target.width), 0,
                                  static_cast<int>(0.5*_overlap * _target.width)};
        aug_pars.translation_y = {static_cast<int>(-0.5*_overlap * _target.height), 0,
                                  static_cast<int>(0.5*_overlap * _target.height)};
        aug_pars.overlap = _overlap;
        Augmentator aug(frame, _target, aug_pars);

        for (auto subframe: aug.SetClass(ObjectClass::Positive)) {
            cv::Mat patch = _make_patch(subframe);
            _add_new_patch(std::move(patch), _positive_sample);
        }

        for (auto subframe: aug.SetClass(ObjectClass::Negative)) {
            cv::Mat patch = _make_patch(subframe);
            _add_new_patch(std::move(patch), _negative_sample);
        }
    }

    void ObjectModel::Train(Candidate candidate) {
        const cv::Mat& frame = *_frame;

        TranformPars aug_pars;
        aug_pars.angles = {-15, 0, 15};
        aug_pars.scales = _scales;
        aug_pars.translation_x = {static_cast<int>(-0.5*_overlap * _target.width), 0,
                                  static_cast<int>(0.5*_overlap * _target.width)};
        aug_pars.translation_x = {static_cast<int>(-0.5*_overlap * _target.height), 0,
                                  static_cast<int>(0.5*_overlap * _target.height)};
        aug_pars.overlap = _overlap;
        Augmentator aug(frame, _target, aug_pars);

        for (auto subframe: aug.SetClass(ObjectClass::Positive)) {
            cv::Mat patch = _make_patch(subframe);
            _add_new_patch(std::move(patch), _positive_sample);
        }

        for (auto subframe: aug.SetClass(ObjectClass::Negative)) {
            cv::Mat patch = _make_patch(subframe);
            _add_new_patch(std::move(patch), _negative_sample);
        }


        ///////////////////////////////////////////
        /*float cval = validate_subframe_const(strobe);
        if (cval< 0.5)//0.65)
        {
            add_positive_example(strobe,0.0,false); //const
        }
        else
        {
            cval = validate_subframe_temp(strobe);
            if (cval< validator_learning_pos_thresh)//0.65)
            {
                add_positive_example(strobe,0.0,true); //temp
            }
        }

        k = 0;
        for (j = 0; j <= stop_y; j = j + step_y)
            for (i = 0; i<=stop_x; i = i + step_x)
            {
                cst.set(i,j,i + szx - 1,j + szy - 1);
                disp = subframe_disp(cst);
                over = overlap(cst,strobe);
                cval = validate_subframe(cst);
                if ((over<=0.1)&&(disp>=target_init_disp*0.25)&&(cval > 0.5))
                {
                    add_negative_example(cst,false);
                    k++;
                }
                if (k>=24)
                {
                   j = src_img_height;
                   i = src_img_width;
                }
            }*/
        //////////////////////////////////////////

    }

    double ObjectModel::Predict(Candidate candidate) {
        const cv::Mat& src_frame = *_frame;
        cv::Mat subframe = src_frame(candidate.strobe);
        auto patch = _make_patch(subframe);
        return _predict(patch);
    }

    void ObjectModel::_add_new_patch(cv::Mat&& patch, std::vector<cv::Mat>& sample) {
        if (_sample_max_depth) {
            if (sample.size() < _sample_max_depth)
                sample.push_back(std::move(patch));
            else {
                size_t idx_2_replace = static_cast<size_t>(get_random_int(static_cast<int>(sample.size() - 1)));
                sample.at(idx_2_replace) = std::move(patch);
            }
        }
    }

    cv::Mat ObjectModel::_make_patch(cv::Mat& subframe) {
        cv::Mat patch;
        cv::resize(subframe, patch, _patch_size);
        return patch;
    }

    double ObjectModel::_similarity_coeff(cv::Mat& patch_0, cv::Mat& patch_1) {
        double ncc = images_correlation(patch_0, patch_1);
        double res = 0.5*(ncc + 1);
        return res;
    }

    double ObjectModel::_sample_dissimilarity(cv::Mat& patch, std::vector<cv::Mat>& sample) {
        double out = 0.0;
        for (auto& sample_patch: sample) {
            out = std::max(_similarity_coeff(sample_patch, patch), out);
        }
        return 1 - out;
    }

    double ObjectModel::_predict(cv::Mat& patch) {
        double out = 0.0;
        double Npm = _sample_dissimilarity(patch, _negative_sample);
        double Ppm = _sample_dissimilarity(patch, _positive_sample);
        if (abs(Npm + Ppm)>1e-9)
            out = Npm / (Npm + Ppm);
        return out;
    }

}
