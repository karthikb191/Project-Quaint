#ifndef _H_AVC_CODEX
#define _H_AVC_CODEX

#include <BMFHelpers.h>
#include <fstream>
#include <iostream>
#include <Interface/IMemoryContext.h>
#include <BMFStructures_Common.h>
#include <BMFParsers.h>
#include <Types/QArray.h>
#include <Types/QFastArray.h>

namespace Quaint { namespace Media
{
    struct AVCDecoderConfigurationRecord;

    struct NALUnit
    {
        NALUnit(int8_t numBytes)
        : m_numBytesInNALUnit(numBytes)
        , m_trailingBytesInRBSP(0)
        {}

        uint8_t getNALRefIDC() { return m_nalRedIdc; }
        uint8_t getNALUnitType() { return m_nalUnitType; }

        virtual void parse(BitParser& parser, const AVCDecoderConfigurationRecord* decoderRecord);

        void dump(const char* byteBuffer, uint16_t length)
        {
            std::cout << "\n";
            for(int i = 0; i < length; i++)
            {
                uint32_t v = (uint8_t)byteBuffer[i];
                std::cout << std::hex << v << " ";
            }
            std::cout << "\n";
        }

    protected:
        uint8_t                     m_numBytesInNALUnit;
        uint8_t                     m_forbiddenZero;
        uint8_t                     m_nalRedIdc;
        uint8_t                     m_nalUnitType;
        uint8_t                     m_trailingBytesInRBSP;

    };

    struct HRDParameters
    {
        HRDParameters(IMemoryContext* context)
        : m_bit_rate_min_1_vals(context)
        , m_cpb_size_min_1_vals(context)
        , m_cbr_flag_vals(context)
        {}
        void parse(BitParser& parser);

        uint32_t            m_cpb_cnt_minus1;
        uint8_t             m_bit_rate_scale;
        uint8_t             m_cpb_size_scale;
        QArray<uint32_t>    m_bit_rate_min_1_vals;
        QArray<uint32_t>    m_cpb_size_min_1_vals;
        QArray<bool>        m_cbr_flag_vals;
        uint8_t             m_initial_cpb_removal_delay_length_minus1;
        uint8_t             m_cpb_removal_delay_length_minus1;
        uint8_t             m_dpb_output_delay_length_minus1;
        uint8_t             m_time_offset_length;
    };
    
    struct VUIParameters
    {
        VUIParameters(IMemoryContext* context)
        : m_nal_hrd_params(context)
        , m_vcl_hrd_params(context)
        {}
        void parse(BitParser& parser, const AVCDecoderConfigurationRecord* decoderRecord);

        bool        m_aspect_ratio_info_present_flag;
        uint8_t     m_aspect_ratio_idc;
        uint16_t    m_sar_width;
        uint16_t    m_sar_height;

        bool        m_overscan_info_present_flag;
        bool        m_overscan_appropriate_flag;
        bool        m_video_signal_type_present_flag;

        uint8_t     m_video_format;
        bool        m_video_full_range_flag;
        bool        m_color_description_present_flag;
        uint8_t     m_color_primaries;
        uint8_t     m_transfer_characteristics;
        uint8_t     m_matrix_coefficients;

        bool        m_chroma_loc_info_present_flag;
        uint32_t    m_chroma_sample_loc_type_top_field;
        uint32_t    m_chroma_sample_loc_type_bottom_field;

        bool        m_timing_info_present_flag;
        uint32_t    m_num_units_in_tick;
        uint32_t    m_time_scale;
        bool        m_fixed_frame_rate_flag;

        bool            m_nal_hrd_parameters_present_flag;
        HRDParameters   m_nal_hrd_params;
        bool            m_vcl_hrd_parameters_present_flag;
        HRDParameters   m_vcl_hrd_params;
        bool            m_low_delay_hrd_flag;
        bool            m_pic_struct_present_flag;
        bool            m_bitstream_restricting_flag;

        bool        m_motion_vectors_over_pic_boundaries_flag;
        uint32_t    m_max_bytes_per_pic_denom;
        uint32_t    m_max_bits_per_mb_denom;
        uint32_t    m_log2_max_mv_length_horizontal;
        uint32_t    m_log2_max_mv_length_vertical;
        uint32_t    m_num_reorder_frames;
        uint32_t    m_dec_frame_buffering;
    };

    struct SequenceParameterSetNALUnit : public NALUnit
    {
        struct SeqExtParams
        {
            enum EScalingListType
            {
                _4X4 = 16,
                _8X8 = 64,
                INVALID
            };

            SeqExtParams(IMemoryContext* context)
            : m_seq_scaling_list_present_flags(context)
            , m_scalingLists_4X4(context)
            , m_scalingLists_8X8(context)
            , m_use_default_scaling_matrix_4X4_flag(context)
            , m_use_default_scaling_matrix_8X8_flag(context)
            {}

            void parse(BitParser& parser, const AVCDecoderConfigurationRecord* decoderRecord);

public:
            uint32_t            m_chroma_format_idc;
            bool                m_separate_colour_plane_flag;
            uint32_t            m_bit_depth_luma_minus8;
            uint32_t            m_bit_depth_chroma_minus8;
            bool                m_qpprime_y_zero_transform_bypass_flag;
            bool                m_seq_scaling_matrix_present_flag;

            QArray<bool>                        m_seq_scaling_list_present_flags;
            QArray<QFastArray<int32_t, 16>>     m_scalingLists_4X4;
            QArray<bool>                        m_use_default_scaling_matrix_4X4_flag;
            QArray<QFastArray<int32_t, 64>>     m_scalingLists_8X8;
            QArray<bool>                        m_use_default_scaling_matrix_8X8_flag;
        };

        SequenceParameterSetNALUnit(IMemoryContext* context)
        : NALUnit(0)
        , m_offsetForRefFrame(context)
        , m_extSeqParams(context)
        , m_vuiParameters(context)
        {}
        SequenceParameterSetNALUnit(IMemoryContext* context, int8_t nalUnitBytes)
        : NALUnit(nalUnitBytes)
        , m_offsetForRefFrame(context)
        , m_extSeqParams(context)
        , m_vuiParameters(context)
        {}

        virtual void parse(BitParser& parser, const AVCDecoderConfigurationRecord* decoderRecord);
        //void parsee(BitParser& parser, const AVCDecoderConfigurationRecord* decoderRecord);

        uint8_t                     m_profileIDC;
        bool                        m_constraintSet_0_flag;
        bool                        m_constraintSet_1_flag;
        bool                        m_constraintSet_2_flag;
        bool                        m_constraintSet_3_flag;
        bool                        m_constraintSet_4_flag;
        bool                        m_constraintSet_5_flag;
        uint8_t                     m_reserved1;
        uint8_t                     m_levelIDC;
        uint32_t                    m_seq_param_set_id;
        
        SeqExtParams                m_extSeqParams;

        uint32_t                    m_log2_max_frame_num_minus4;
        uint32_t                    m_pic_order_cnt_type;
        uint32_t                    m_log2_max_pic_order_cnt_lsb_minus4;
        bool                        m_delta_pic_order_always_zero_flag;
        int32_t                     m_offset_for_non_ref_pic;
        int32_t                     m_offset_for_top_to_bottom_field;
        uint32_t                    m_num_ref_frames_in_pic_order_cnt_cycle;
        QArray<int32_t>             m_offsetForRefFrame;
        
        uint32_t                    m_max_num_ref_frames;
        bool                        m_gaps_in_frame_num_value_allowed_flag;
        uint32_t                    m_pic_width_in_mbs_minus1;
        uint32_t                    m_pic_height_in_map_units_minus1;
        bool                        m_frame_mbs_only_flag;
        bool                        m_mb_adaptive_frame_field_flag;
        bool                        m_direct_8x8_inference_flag;
        bool                        m_frame_cropping_flag;
        uint32_t                    m_frame_crop_left_offset;
        uint32_t                    m_frame_crop_right_offset;
        uint32_t                    m_frame_crop_top_offset;
        uint32_t                    m_frame_crop_bottom_offset;
        bool                        m_vui_parameters_present_flag;

        VUIParameters               m_vuiParameters;
    };

    struct PictureParameterSetNALUnit : public NALUnit
    {
        PictureParameterSetNALUnit(IMemoryContext* context)
        : PictureParameterSetNALUnit(context, 0)
        {}
        PictureParameterSetNALUnit(IMemoryContext* context, int8_t nalUnitBytes)
        : NALUnit(nalUnitBytes)
        , m_run_length_minus1(context)
        , m_top_left(context)
        , m_bottom_right(context)
        , m_sice_group_id(context)
        , m_pic_scaling_list_present_flags(context)
        , m_scalingLists_4X4(context)
        , m_use_default_scaling_matrix_4X4_flag(context)
        , m_scalingLists_8X8(context)
        , m_use_default_scaling_matrix_8X8_flag(context)
        {}

        virtual void parse(BitParser& parser, const AVCDecoderConfigurationRecord* decoderRecord) override;

        uint32_t                        m_pic_parameter_set_id;
        uint32_t                        m_seq_parameter_set_id;
        bool                            m_entropy_coding_mode_flag;
        bool                            m_bottom_field_pic_order_in_frame_present_flag;
        uint32_t                        m_num_slice_groups_minus1;
        uint32_t                        m_slice_group_map_type;

        QArray<uint32_t>                m_run_length_minus1;
        QArray<uint32_t>                m_top_left;
        QArray<uint32_t>                m_bottom_right;

        bool                            m_slice_group_change_direction_flag;
        uint32_t                        m_slice_group_change_rate_minus1;
        uint32_t                        m_pic_size_in_map_units_minus1;
        QArray<uint32_t>                m_sice_group_id;

        uint32_t                        m_num_ref_idx_l0_default_active_minus1;
        uint32_t                        m_num_ref_idx_l1_default_active_minus1;
        bool                            m_weighted_pred_flag;
        uint8_t                         m_weighted_bipred_idc;
        int32_t                         m_pic_init_qp_minus26;
        int32_t                         m_pic_init_qs_minus26;
        int32_t                         m_chroma_qp_index_offset;
        bool                            m_deblocking_filter_control_present_flag;
        bool                            m_constrained_intra_pred_flag;
        bool                            m_redundant_pic_cnt_present_flag;

        bool                            m_transform_8x8_mode_flag;
        bool                            m_pic_scaling_matrix_present_flag;
        QArray<bool>                    m_pic_scaling_list_present_flags;
        QArray<QFastArray<int32_t, 16>> m_scalingLists_4X4;
        QArray<bool>                    m_use_default_scaling_matrix_4X4_flag;
        QArray<QFastArray<int32_t, 64>> m_scalingLists_8X8;
        QArray<bool>                    m_use_default_scaling_matrix_8X8_flag;
        int32_t                         m_second_chroma_qp_index_offset;
    };

    struct AVCDecoderConfigurationRecord
    {
        AVCDecoderConfigurationRecord(IMemoryContext* context)
        : m_sequenceParamSets(context, context)
        , m_pictureParamSets(context, context)
        {}

        uint8_t                                 m_version;
        uint8_t                                 m_avcProfileIndication;
        uint8_t                                 m_profileCompatibility;
        uint8_t                                 m_avcLevelIndication;
        uint8_t                                 m_nalUnitLength; /*Modifying this to actually hold the correct value of NAL unit when parsing. Actual val is 2 bits*/
        uint8_t                                 m_numSequenceParamSets; /*Modifying this to actually hold the correct value of NAL unit when parsing. Actual val is 5 bits*/
        QArray<SequenceParameterSetNALUnit>     m_sequenceParamSets;
        QArray<PictureParameterSetNALUnit>      m_pictureParamSets;
        uint8_t                                 m_numPictureParamSets;

    };

    struct AVCConfigurationBox : public Box
    {
        AVCConfigurationBox(IMemoryContext* context)
        : m_decoderRecord(context)
        {}
        AVCDecoderConfigurationRecord       m_decoderRecord;
    };

}}

#endif //_H_AVC_CODEX