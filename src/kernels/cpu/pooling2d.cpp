#include <kernels/cpu/pooling2d.h>
#include <core/tensor_builder.h>
#include "backend/common_function.h"
#include "backend/name.h"
#include "utils/assert.h"

namespace ts {

	Pooling2d::Pooling2d() 
	{
		field(name::format, REQUIRED);
		field(name::type, REQUIRED);
		field(name::padding, REQUIRED);
		Tensor default_padding_type = tensor::from<int32_t>(PDDINGTYPE::black);
		field(name::padding_type, OPTIONAL, default_padding_type);
		field(name::ksize, REQUIRED);
		field(name::stride, REQUIRED);
	}

	void Pooling2d::init()
	{
		supper::init();

		m_format = tensor::to_string(this->get(name::format));
		TS_AUTO_CHECK(m_format == name::NCHW || m_format == name::NHWC);
		TS_AUTO_CHECK(m_format == name::NCHW);    // only support NCHW now

		auto static_padding = tensor::cast(INT32, this->get(name::padding));
		TS_AUTO_CHECK(static_padding.has_shape({ 4,2 }));
		//TS_AUTO_CHECK(static_padding.dims() == 2 && static_padding.size(0) == 4 && static_padding.size(1) == 2);

		auto ksize_tensor = tensor::cast(INT32, this->get(name::ksize));
		TS_AUTO_CHECK(ksize_tensor.has_shape({ 4 }));
		//TS_AUTO_CHECK(ksize_tensor.dims() == 1 && ksize_tensor.count() == 4);

		auto stride_tensor = tensor::cast(INT32, this->get(name::stride));
		TS_AUTO_CHECK(stride_tensor.has_shape({ 4 }));
		//TS_AUTO_CHECK(stride_tensor.dims() == 1 && stride_tensor.count() == 4);

		if (m_format == name::NCHW)
		{
			TS_AUTO_CHECK(static_padding.data<int32_t>()[0] == 0 && static_padding.data<int32_t>()[1] == 0
				&& static_padding.data<int32_t>()[2] == 0 && static_padding.data<int32_t>()[3] == 0);
			TS_AUTO_CHECK(ksize_tensor.data<int32_t>()[0] == 1 && ksize_tensor.data<int32_t>()[1] == 1);
			TS_AUTO_CHECK(stride_tensor.data<int32_t>()[0] == 1 && stride_tensor.data<int32_t>()[1] == 1);
			m_padding.top = static_padding.data<int32_t>()[4];
			m_padding.bottom = static_padding.data<int32_t>()[5];
			m_padding.left = static_padding.data<int32_t>()[6];
			m_padding.right = static_padding.data<int32_t>()[7];
			m_ksize.height = ksize_tensor.data<int32_t>()[2];
			m_ksize.width = ksize_tensor.data<int32_t>()[3];
			m_stride.height = stride_tensor.data<int32_t>()[2];
			m_stride.width = stride_tensor.data<int32_t>()[3];
		}
		else
		{
			TS_AUTO_CHECK(static_padding.data<int32_t>()[0] == 0 && static_padding.data<int32_t>()[1] == 0
				&& static_padding.data<int32_t>()[6] == 0 && static_padding.data<int32_t>()[7] == 0);
			TS_AUTO_CHECK(ksize_tensor.data<int32_t>()[0] == 1 && ksize_tensor.data<int32_t>()[3] == 1);
			TS_AUTO_CHECK(stride_tensor.data<int32_t>()[0] == 1 && stride_tensor.data<int32_t>()[3] == 1);
			m_padding.top = static_padding.data<int32_t>()[2];
			m_padding.bottom = static_padding.data<int32_t>()[3];
			m_padding.left = static_padding.data<int32_t>()[4];
			m_padding.right = static_padding.data<int32_t>()[5];
			m_ksize.height = ksize_tensor.data<int32_t>()[1];
			m_ksize.width = ksize_tensor.data<int32_t>()[2];
			m_stride.height = stride_tensor.data<int32_t>()[1];
			m_stride.width = stride_tensor.data<int32_t>()[2];
		}

		ts::Tensor& padding_type_param = get(name::padding_type);
		m_padding_type = (PDDINGTYPE)tensor::to_int(padding_type_param);


		TS_AUTO_CHECK(m_padding_type == black);

		m_pooling_type = (POOLINGTYPE)tensor::to_int(get(name::type));
	}

	int Pooling2d::run(ts::Stack &stack)
	{
		int input_num = stack.size();
		std::vector<ts::Tensor::Prototype> output;

		this->infer(stack, output);
		stack.push(output[0], MemoryDevice(CPU));

		auto dtype = stack.index(0)->dtype();
		switch (dtype) {
	#define DECLARE_TYPE_AND_RUN(DTYPE, TYPE) \
					case DTYPE: { pooling<TYPE>(stack); break; }
				DECLARE_TYPE_AND_RUN(INT8, int8_t);
				DECLARE_TYPE_AND_RUN(UINT8, uint8_t);
				DECLARE_TYPE_AND_RUN(INT16, int16_t);
				DECLARE_TYPE_AND_RUN(UINT16, uint16_t);
				DECLARE_TYPE_AND_RUN(INT32, int32_t);
				DECLARE_TYPE_AND_RUN(UINT32, uint32_t);
				DECLARE_TYPE_AND_RUN(INT64, int64_t);
				DECLARE_TYPE_AND_RUN(UINT64, uint64_t);
				DECLARE_TYPE_AND_RUN(FLOAT32, float);
				DECLARE_TYPE_AND_RUN(FLOAT64, double);
	#undef DECLARE_TYPE_AND_RUN
			default: {
				TS_LOG_ERROR << "pooling2d not support this data type: " << dtype << eject;
				break;
			}
		}
		return 1;
	}

	int Pooling2d::infer(ts::Stack &stack, std::vector<ts::Tensor::Prototype> &output)
	{
		TS_AUTO_CHECK(stack.size() == 1 && stack.index(0)->dims() == 4);
		TS_AUTO_CHECK(stack.index(0)->dtype() == FLOAT32 || stack.index(0)->dtype() == FLOAT64);

		Size2D input_size;

		if (m_format == name::NCHW)
		{
			input_size.height = stack.index(0)->sizes()[2];
			input_size.width = stack.index(0)->sizes()[3];
		}
		else
		{
			input_size.height = stack.index(0)->sizes()[1];
			input_size.width = stack.index(0)->sizes()[2];
		}

		Size2D output_size;
		output_size = pooling2d_forward(input_size, m_padding, m_ksize, m_stride);

		Shape output_shape(stack.index(0)->sizes());
		if (m_format == name::NCHW)
		{
			output_shape[2] = output_size.height;
			output_shape[3] = output_size.width;
		}
		else
		{
			output_shape[1] = output_size.height;
			output_shape[2] = output_size.width;
		}
		output.resize(1);
		output[0] = ts::Tensor::Prototype(stack.index(0)->dtype(), output_shape);
		return 1;
	}

	template<typename T>
	bool Pooling2d::pooling(ts::Stack &stack)
	{
		ts::Tensor input_tensor = *stack.index(0);
		Shape input_shape = input_tensor.sizes();
		T* input_data = input_tensor.sync(MemoryDevice(CPU)).data<T>();

		ts::Tensor& output_tensor = *stack.index(-1);
		Shape output_shape = output_tensor.sizes();
		T* output_data = output_tensor.data<T>();

		bool flag;

		if (m_pooling_type == max)
		{
			flag = max_pooling<T>(input_data, output_data, input_shape, output_shape, m_ksize, m_stride);
		}
		else
		{
			flag = average_pooling<T>(input_data, output_data, input_shape, output_shape, m_ksize, m_stride);
		}
		return flag;
	}

	template<typename T>
	bool Pooling2d::max_pooling(T* input_data, T* output_data, Shape& input_shape, Shape& output_shape, KSize2D& ksize, Stride2D& stride)
	{
		int input_h = input_shape[2];
		int input_w = input_shape[3];
		int output_h = output_shape[2];
		int output_w = output_shape[3];
		int input_channel_size = input_h * input_w;
		int output_channel_size = output_h * output_w;
		for (int n = 0; n < output_shape[0]; n++)
		{
			for (int c = 0; c< output_shape[1]; c++)
			{
				for (int oh = 0; oh < output_shape[2]; oh++)
				{
					int ihStart = oh * stride.height - m_padding.top;
					int ihEnd = std::min<T>(ihStart + ksize.height, input_h);
					for (int ow = 0; ow < output_shape[3]; ow++)
					{		
						int iwStart = ow * stride.width - m_padding.left;
						int iwEnd = std::min<T>(iwStart + ksize.width, input_w);
						ihStart = std::max<T>(ihStart, 0);
						iwStart = std::max<T>(iwStart, 0);
						int outIndex = oh * output_w + ow;
						T maxVlue = 0;
						//int count = 0;
						for (int ih = ihStart; ih < ihEnd; ih++)
						{
							for (int iw = iwStart; iw < iwEnd; iw++)
							{
								int input_index = ih * input_w + iw;
								if (input_data[input_index] > maxVlue)
								{
									maxVlue = input_data[input_index];
								}
							}
							//count++;
						}
						output_data[outIndex] = maxVlue;
						//if (count == m_kernel_h * m_kernel_w)
						//	output_data[outIndex] = maxVlue;
						//else
						//	output_data[outIndex] = std::max<T>(maxVlue, padding_value);
					}
				}
				input_data += input_channel_size;
				output_data += output_channel_size;
			}
		}
		return true;
	}

	template<typename T>
	bool Pooling2d::average_pooling(T* input_data, T* output_data, Shape& input_shape, Shape& output_shape, KSize2D& ksize, Stride2D& stride)
	{
		int input_h = input_shape[2];
		int input_w = input_shape[3];
		int output_h = output_shape[2];
		int output_w = output_shape[3];
		int input_channel_size = input_h * input_w;
		int output_channel_size = output_h * output_w;
		for (int n = 0; n < output_shape[0]; n++)
		{
			for (int c = 0; c< output_shape[1]; c++)
			{
				for (int oh = 0; oh < output_shape[2]; oh++)
				{
					int ihStart = oh * stride.height - m_padding.top;
					int ihEnd = std::min<T>(ihStart + ksize.height, input_h);
					for (int ow = 0; ow < output_shape[3]; ow++)
					{
						int iwStart = ow * stride.width - m_padding.left;	
						int iwEnd = std::min<T>(iwStart + ksize.width, input_w);
						ihStart = std::max<T>(ihStart, 0);
						iwStart = std::max<T>(iwStart, 0);
						int outIndex = oh * output_w + ow;
						T sumValue = 0.0;
						int count = 0;
						for (int ih = ihStart; ih < ihEnd; ih++)
						{
							for (int iw = iwStart; iw < iwEnd; iw++)
							{
								int input_index = ih * input_w + iw;
								sumValue += input_data[input_index];
								count++;
							}
						}
						if (count == 0)
							output_data[outIndex] = 0;
						else
							output_data[outIndex] = sumValue / count;
						//if (count == 0)
						//	output_data[outIndex] = 0;
						//else if (count == m_kernel_h * m_kernel_w)
						//	output_data[outIndex] = sumValue / count;
						//else
						//	output_data[outIndex] = (sumValue + (m_kernel_h * m_kernel_w - count) * padding_value) / (m_kernel_h * m_kernel_w);
					}
				}
				input_data += input_channel_size;
				output_data += output_channel_size;
			}
		}
		return true;
	}
}

using namespace ts;
TS_REGISTER_OPERATOR(Pooling2d, ts::CPU, name::layer::pooling2d())