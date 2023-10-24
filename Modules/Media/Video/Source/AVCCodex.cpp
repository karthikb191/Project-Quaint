#include <AVCCodex.h>
#include <assert.h>
#include <QuaintLogger.h>

namespace Quaint {namespace Media{

    /** Parse Functions*/

    DECLARE_LOG_CATEGORY(AVCCodex);
    DEFINE_LOG_CATEGORY(AVCCodex);

    void parse_rbsp_trailing_bits(BitParser& parser)
    {
        int8_t r = (uint8_t)parser.readBits(1);
        assert(r == 1 && "RBSP parse invalid");
        parser.alignToByte();
    }

    template<int32_t SZ>
    void parseScalingLists(BitParser& parser, 
    QFastArray<int32_t, SZ> scalingList, uint32_t sizeOfScalingList, 
    bool& useDefaultScalingMatrixFlag)
    {
        assert(SZ == 16 || SZ == 64 && "Invalid scaling list passed");
        int32_t lastScale = 8;
        int32_t nextScale = 8;
        for(uint32_t j = 0; j < sizeOfScalingList; j++ )
        { 
            if( nextScale != 0 ) 
            { 
                int32_t delta_scale = parser.se();
                nextScale = ( lastScale + delta_scale + 256 ) % 256;
                useDefaultScalingMatrixFlag = ( j == 0 && nextScale == 0 ); 
            } 
            scalingList[j] = ( nextScale == 0 ) ? lastScale : nextScale;
            lastScale = scalingList[j]; 
        }
    }

    void NALUnit::parse(BitParser& parser, const AVCDecoderConfigurationRecord* decoderRecord)
    {
        m_forbiddenZero = parser.readBits(1);
        m_nalRedIdc = parser.readBits(2);
        m_nalUnitType = parser.readBits(5);

        for(int8_t i = 1; i < m_numBytesInNALUnit; i++ ) {
            uint32_t r = parser.nextBits(24);
            if( i + 2 < m_numBytesInNALUnit && parser.nextBits(24) == 0x000003 ) 
            {
                ++m_trailingBytesInRBSP;
                ++m_trailingBytesInRBSP;
                i += 2;
            } 
            else
            {
                ++m_trailingBytesInRBSP;
            }
        }
    }

    void SequenceParameterSetNALUnit::SeqExtParams::parse(BitParser& parser, const AVCDecoderConfigurationRecord* decoderRecord)
    {
        m_chroma_format_idc = parser.ue();
        if(m_chroma_format_idc == 3)
        {
            m_separate_colour_plane_flag = (bool)parser.readBits(1);
        }

        m_bit_depth_luma_minus8 = parser.ue();
        m_bit_depth_chroma_minus8 = parser.ue();
        m_qpprime_y_zero_transform_bypass_flag = (bool)parser.readBits(1);
        m_seq_scaling_matrix_present_flag = (bool)parser.readBits(1);

        if(m_seq_scaling_matrix_present_flag)
        {
            uint32_t length = ((m_chroma_format_idc != 3) ? 8 : 12 );
            m_seq_scaling_list_present_flags.resize(length);
            m_scalingLists_4X4.resize(length);
            m_use_default_scaling_matrix_4X4_flag.resize(length);
            m_scalingLists_8X8.resize(length);
            m_use_default_scaling_matrix_8X8_flag.resize(length);
            
            for(uint32_t i = 0; i < length; i++ )
            {
                m_seq_scaling_list_present_flags[i] = (bool)parser.readBits(1);
                if(m_seq_scaling_list_present_flags[i])
                {
                    if( i < 6 )
                    {
                        parseScalingLists(parser, m_scalingLists_4X4[i], 16, m_use_default_scaling_matrix_4X4_flag[i]);
                    }
                    else
                    {
                        parseScalingLists(parser, m_scalingLists_8X8[i - 6], 64, m_use_default_scaling_matrix_8X8_flag[i - 6]);
                    }
                }
            }
        }
    }

    void HRDParameters::parse(BitParser& parser)
    {
        m_cpb_cnt_minus1 = parser.ue();
        m_bit_rate_scale = (uint8_t)parser.readBits(4);
        m_cpb_size_scale = (uint8_t)parser.readBits(4);
        
        m_bit_rate_min_1_vals.resize(m_cpb_cnt_minus1 + 1);
        m_cpb_size_min_1_vals.resize(m_cpb_cnt_minus1 + 1);
        m_cbr_flag_vals.resize(m_cpb_cnt_minus1 + 1);
        for(uint32_t i = 0; i <= m_cpb_cnt_minus1; i++)
        {
            m_bit_rate_min_1_vals[i] = parser.ue();
            m_cpb_size_min_1_vals[i] = parser.ue();
            m_cbr_flag_vals[i] = (bool)parser.readBits(1);
        }
        m_initial_cpb_removal_delay_length_minus1 = (uint8_t)parser.readBits(5);
        m_cpb_removal_delay_length_minus1 = (uint8_t)parser.readBits(5);
        m_dpb_output_delay_length_minus1 = (uint8_t)parser.readBits(5);
        m_time_offset_length = (uint8_t)parser.readBits(5);
    }

    void VUIParameters::parse(BitParser& parser, const AVCDecoderConfigurationRecord* decoderRecord)
    {
        m_aspect_ratio_info_present_flag = (bool)parser.readBits(1);
        if(m_aspect_ratio_info_present_flag)
        {
            m_aspect_ratio_idc = (uint8_t)parser.readBits(8);
            if(m_aspect_ratio_idc == 255)
            {
                m_sar_width = (uint16_t)parser.readBits(16);
                m_sar_height = (uint16_t)parser.readBits(16);
            }
        }

        m_overscan_info_present_flag = (bool)parser.readBits(1);
        if(m_overscan_info_present_flag)
        {
            m_overscan_appropriate_flag = (bool)parser.readBits(1);
        }

        m_video_signal_type_present_flag = (bool)parser.readBits(1);
        if(m_video_signal_type_present_flag)
        {
            m_video_format = (uint8_t)parser.readBits(3);
            m_video_full_range_flag = (bool)parser.readBits(1);
            m_color_description_present_flag = (bool)parser.readBits(1);
            if(m_color_description_present_flag)
            {
                m_color_primaries = (uint8_t)parser.readBits(8);
                m_transfer_characteristics = (uint8_t)parser.readBits(8);
                m_matrix_coefficients = (uint8_t)parser.readBits(8);
            }
        }

        m_chroma_loc_info_present_flag = (bool)parser.readBits(1);
        if(m_chroma_loc_info_present_flag)
        {
            m_chroma_sample_loc_type_top_field = parser.ue();
            m_chroma_sample_loc_type_bottom_field = parser.ue();
        }

        m_timing_info_present_flag = (bool)parser.readBits(1);
        if(m_timing_info_present_flag)
        {
            m_num_units_in_tick = parser.readBits(32);
            m_time_scale = parser.readBits(32);
            m_fixed_frame_rate_flag = (bool)parser.readBits(1);
        }

        m_nal_hrd_parameters_present_flag = (bool)parser.readBits(1);
        if(m_nal_hrd_parameters_present_flag)
        {
            m_nal_hrd_params.parse(parser);
        }
        m_vcl_hrd_parameters_present_flag = (bool)parser.readBits(1);
        if(m_vcl_hrd_parameters_present_flag)
        {
            m_vcl_hrd_params.parse(parser);
        }
        if(m_nal_hrd_parameters_present_flag || m_vcl_hrd_parameters_present_flag)
        {
            m_low_delay_hrd_flag = (bool)parser.readBits(1);
        }

        m_pic_struct_present_flag = (bool)parser.readBits(1);

        if(parser.isComplete()) return;

        m_bitstream_restricting_flag = (bool)parser.readBits(1);
        if(m_bitstream_restricting_flag)
        {
            m_motion_vectors_over_pic_boundaries_flag = (bool)parser.readBits(1);
            m_max_bytes_per_pic_denom = parser.ue();
            m_max_bits_per_mb_denom = parser.ue();
            m_log2_max_mv_length_horizontal = parser.ue();
            m_log2_max_mv_length_vertical = parser.ue();
            m_num_reorder_frames = parser.ue();
            m_dec_frame_buffering = parser.ue();

            if(parser.isOverflown())
            {
                m_bitstream_restricting_flag = false;
                m_num_reorder_frames = 0;
            }
        }
    }

    void SequenceParameterSetNALUnit::parse(BitParser& parser, const AVCDecoderConfigurationRecord* decoderRecord)
    {
        NALUnit::parse(parser, decoderRecord);
        assert(m_nalUnitType == 7 && "Invalid NAL unit type. Sequence parameter set requires NAL unit type 7");

        m_profileIDC = (int8_t)parser.readBits(8);
        m_constraintSet_0_flag = parser.readBits(1);
        m_constraintSet_1_flag = parser.readBits(1);
        m_constraintSet_2_flag = parser.readBits(1);
        m_constraintSet_3_flag = parser.readBits(1);
        m_constraintSet_4_flag = parser.readBits(1);
        m_constraintSet_5_flag = parser.readBits(1);
        m_reserved1 = parser.readBits(2);

        m_levelIDC = parser.readBits(8);
        m_seq_param_set_id = parser.ue();

        if(m_profileIDC == 100 || m_profileIDC == 110 || m_profileIDC == 122 || m_profileIDC == 244 || 
        m_profileIDC == 44 || m_profileIDC == 83 || m_profileIDC == 86 || m_profileIDC == 118 || m_profileIDC == 128 
        || m_profileIDC == 138 || m_profileIDC == 139 || m_profileIDC == 134 || m_profileIDC == 135)
        {
            m_extSeqParams.parse(parser, decoderRecord);
        }

        m_log2_max_frame_num_minus4 = parser.ue();
        m_pic_order_cnt_type = parser.ue();

        if(m_pic_order_cnt_type == 0)
        {
            m_log2_max_pic_order_cnt_lsb_minus4 = parser.ue();
        }
        else if(m_pic_order_cnt_type == 1)
        {
            m_delta_pic_order_always_zero_flag = parser.readBits(1);
            m_offset_for_non_ref_pic = parser.se();
            m_offset_for_top_to_bottom_field = parser.se();
            m_num_ref_frames_in_pic_order_cnt_cycle = parser.ue();
            for(uint32_t i = 0; i < m_num_ref_frames_in_pic_order_cnt_cycle; ++i)
            {
                m_offsetForRefFrame.pushBack(parser.se());
            }
        }

        m_max_num_ref_frames = parser.ue();
        m_gaps_in_frame_num_value_allowed_flag = (bool)parser.readBits(1);
        m_pic_width_in_mbs_minus1 = parser.ue();
        m_pic_height_in_map_units_minus1 = parser.ue();
        m_frame_mbs_only_flag = parser.readBits(1);
        if(!m_frame_mbs_only_flag)
        {
            m_mb_adaptive_frame_field_flag = parser.readBits(1);
        }

        m_direct_8x8_inference_flag = parser.readBits(1);
        m_frame_cropping_flag = parser.readBits(1);
        if(m_frame_cropping_flag)
        {
            m_frame_crop_left_offset = parser.ue();
            m_frame_crop_right_offset = parser.ue();
            m_frame_crop_top_offset = parser.ue();
            m_frame_crop_bottom_offset = parser.ue();
        }
        m_vui_parameters_present_flag = parser.readBits(1);

        assert(!parser.isOverflown() && "Parser overflown at unexpected location. The file might be incorrectly parsed");
        if(parser.isOverflown()) return;

        if(m_vui_parameters_present_flag)
        {
            m_vuiParameters.parse(parser, decoderRecord);
        }

        if(parser.isOverflown())
        {
            QLOG_W(AVCCodex, "Parser overflown after parsing VUI params. Shouldn't affect slive decoding process according to docs");
            return;
        }

        parse_rbsp_trailing_bits(parser);
        
        assert(parser.isComplete() && "Parse incomplete. This may lead to issues when decoding");
    }

    void PictureParameterSetNALUnit::parse(BitParser& parser, const AVCDecoderConfigurationRecord* decoderRecord)
    {
        NALUnit::parse(parser, decoderRecord);
        //assert(m_nalUnitType == 7 && "Invalid NAL unit type. Sequence parameter set requires NAL unit type 7");

        m_pic_parameter_set_id = (int8_t)parser.ue();
        m_seq_parameter_set_id = (int8_t)parser.ue();
        m_entropy_coding_mode_flag = parser.readBits(1);
        m_bottom_field_pic_order_in_frame_present_flag = parser.readBits(1);
        m_num_slice_groups_minus1 = parser.ue();
        if(m_num_slice_groups_minus1 > 0)
        {
            m_slice_group_map_type = parser.ue();
            if(m_slice_group_map_type == 0)
            {
                m_run_length_minus1.resize(m_num_slice_groups_minus1 + 1);
                for(uint32_t i = 0; i <= m_num_slice_groups_minus1; i++)
                {
                    m_run_length_minus1[i] = parser.ue();
                }
            }
            else if(m_slice_group_map_type == 2)
            {
                m_top_left.resize(m_num_slice_groups_minus1 + 1);
                m_bottom_right.resize(m_num_slice_groups_minus1 + 1);
                for(uint32_t i = 0; i <= m_num_slice_groups_minus1; i++)
                {
                    m_top_left[i] = parser.ue();
                    m_bottom_right[i] = parser.ue();
                }
            }
            else if(m_slice_group_map_type == 3 
            || m_slice_group_map_type == 4
            || m_slice_group_map_type == 5)
            {
                m_slice_group_change_direction_flag = (bool)parser.readBits(1);
                m_slice_group_change_rate_minus1 = parser.ue();
            }
            else if(m_slice_group_map_type == 6)
            {
                m_pic_size_in_map_units_minus1 = parser.ue();
                m_sice_group_id.resize(m_pic_size_in_map_units_minus1 + 1);
                for(uint32_t i = 0; i <= m_pic_size_in_map_units_minus1; i++)
                {
                    uint32_t length = (uint32_t)ceil(log2(m_num_slice_groups_minus1 + 1));
                    m_sice_group_id[i] = parser.readBits(length);
                }
            }
        }

        m_num_ref_idx_l0_default_active_minus1 = parser.ue();
        m_num_ref_idx_l1_default_active_minus1 = parser.ue();
        m_weighted_pred_flag = parser.readBits(1);
        m_weighted_bipred_idc = (uint8_t)parser.readBits(2);

        //TODO: These might need some additional handling according to ffmpeg
        //TODO: There are some QP tables being built after getting this data. Investigate
        m_pic_init_qp_minus26 = parser.se();
        m_pic_init_qs_minus26 = parser.se();
        m_chroma_qp_index_offset = parser.se();
        m_deblocking_filter_control_present_flag = parser.readBits(1);
        m_constrained_intra_pred_flag = parser.readBits(1);
        m_redundant_pic_cnt_present_flag = parser.readBits(1);

        if(parser.moreRBSPDataExists())
        {
            m_transform_8x8_mode_flag = parser.readBits(1);
            m_pic_scaling_matrix_present_flag = parser.readBits(1);
            if(m_pic_scaling_matrix_present_flag)
            {
                uint32_t chroma_format_idc = decoderRecord->m_sequenceParamSets.get(m_seq_parameter_set_id).m_extSeqParams.m_chroma_format_idc;
                uint32_t length = 6 + ((chroma_format_idc != 3) ? 2 : 6 ) * m_transform_8x8_mode_flag;
                m_pic_scaling_list_present_flags.resize(length);
                m_scalingLists_4X4.resize(length);
                m_use_default_scaling_matrix_4X4_flag.resize(length);
                m_scalingLists_8X8.resize(length);
                m_use_default_scaling_matrix_8X8_flag.resize(length);
                for(uint32_t i = 0; i < length; i++)
                {
                    m_pic_scaling_list_present_flags[i] = parser.readBits(1);
                    if(m_pic_scaling_list_present_flags[i])
                    {
                        if( i < 6)
                        {
                            parseScalingLists(parser, m_scalingLists_4X4[i], 16, m_use_default_scaling_matrix_4X4_flag[i]);
                        }
                        else
                        {
                            parseScalingLists(parser, m_scalingLists_8X8[i], 64, m_use_default_scaling_matrix_8X8_flag[i]);
                        }
                    }
                }
            }
            m_second_chroma_qp_index_offset = parser.se();
        }
        if(!parser.isComplete() && !parser.isOverflown())
        {
            parse_rbsp_trailing_bits(parser);
        }
    }

}}